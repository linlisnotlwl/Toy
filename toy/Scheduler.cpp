#include "Scheduler.h"

namespace Toy
{


Scheduler::Scheduler() : m_timer(std::make_shared<TimerWheel>())
{
    m_all_handlers.push_back(new Cohandler(this, 0));
}

Scheduler::~Scheduler()
{
    stop();
    for(auto &p : m_all_handlers)
        delete p;
    m_all_handlers.clear();
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
        is_running = true;

        m_timer->start();
        m_timer->autoUpdate();
        
        size_t temp = std::thread::hardware_concurrency();
        if(thread_num == 0 || thread_num > temp)
            thread_num = temp; 
            //thread_num = 1; //just fot test
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
    if(co->getCohandler() != nullptr)
    {
        co->getCohandler()->addCoroutine(co);
        return;
    }

    auto cohandler = Cohandler::getCurHandler();
    if(cohandler && cohandler->getCurScheduler() == this)
    {
        cohandler->addCoroutine(co);
        return; 
    }

    auto min_it = m_all_handlers.begin();
    uint64_t cur_min = 0;
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