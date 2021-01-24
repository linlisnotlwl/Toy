#include "Scheduler.h"
#include "Util.h"
#include "Hook.h"
namespace Toy
{


Scheduler::Scheduler() : m_timer(std::make_shared<TimerWheel>())
{
    // 保证后面添加Coroutine时，可以被添加到Cohandler中
    m_all_handlers.push_back(new Cohandler(this, 0));
}

Scheduler::~Scheduler()
{
    stop();
    for(auto &p : m_all_handlers)
        delete p;
    m_all_handlers.clear();
    m_timer->close();
}

void Scheduler::createCoroutine(const CoFunction & cf, size_t stack_size)
{
    Coroutine * co = new Coroutine(cf, stack_size);
    addCoroutine(co);
}

void Scheduler::start(size_t thread_num, bool using_cur_thread)
{ 
    
    if(!is_running)
    {
        enableHook();
        is_running = true;

        m_timer->start();
        m_timer->autoUpdate();
        
        size_t temp = std::thread::hardware_concurrency();
        if(thread_num == 0 || thread_num > temp)
            thread_num = temp; 
        thread_num = 1; //just fot test
        {
            std::unique_lock<std::mutex> lock(m_handlers_mutex);
            for(size_t i = 1; i < thread_num; ++i)
            {
                Cohandler * p = new Cohandler(this, i);
                std::thread t([this, p](){ p->handler(); });
                t.detach();
                m_all_handlers.push_back(p);
            }
        }
        
        //TODO: Scheduler::dispatch() dispatch the cur threads

        //std::thread t([this](){ this->m_all_handlers.front()->handler(); });
        //t.detach();
        if(using_cur_thread)
            m_all_handlers.front()->handler();
        else
        {
            std::thread t([this](){ this->m_all_handlers.front()->handler(); });
            t.detach();
        }
        
    }

}

void Scheduler::stop()
{
    
    if(is_running)
    {   
        // cohandler 会检测is_running 所有要先设置为false
        is_running = false;
        // TODO

        std::unique_lock<std::mutex> lock(m_handlers_mutex);
        for(auto & p : m_all_handlers)
            p->m_sema.notify();
        disableHook();
    }
}

size_t Scheduler::getCoNum()
{
    //TODO
    return 0;
}

void Scheduler::dispatch()
{
    //TODO
}

void Scheduler::addCoroutine(Coroutine * co)
{
    auto cohandler = co->getCohandler();
    if(cohandler != nullptr )//不一定是跑着&& cohandler->is_running
    {
        cohandler->addCoroutine(co);
        return;
    }

    cohandler = Cohandler::getCurHandler();
    if(cohandler && cohandler->getCurScheduler() == this)//&& cohandler->is_running
    {
        cohandler->addCoroutine(co);
        return; 
    }

    // 找到Coroutine数量最少的Cohandler
    auto min_it = m_all_handlers.begin();
    TOY_ASSERT(min_it != m_all_handlers.end());
    uint64_t cur_min = (*min_it)->m_co_count;
    for (auto it = m_all_handlers.begin(); it != m_all_handlers.end(); ++it)
    {
        if ((*it)->m_co_count < cur_min)
        {
            cur_min = (*it)->m_co_count;
            min_it = it;
        }
    }

    (*min_it)->addCoroutine(co);
}

} // namespace Toy