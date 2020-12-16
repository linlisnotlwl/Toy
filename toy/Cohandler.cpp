#include "Cohandler.h"
#include "Util.h"

namespace Toy
{

Cohandler::Cohandler(Scheduler * sd, int id)
    : m_scheduler(sd), m_id(id)
{
    
}

Cohandler::~Cohandler()
{
    m_sema.notify();
    for(auto & p : m_waiting_cos)
        delete p;
    m_waiting_cos.clear();
    for(auto & p : m_new_cos)
        delete p;
    m_new_cos.clear();
    for(auto & p : m_runnable_cos)
        delete p;
    m_runnable_cos.clear();
}

Cohandler * & Cohandler::getCurHandler()
{
    static thread_local Cohandler * cur_cohandler = nullptr;
    return cur_cohandler;
}

Scheduler * Cohandler::getCurScheduler()
{
    auto p = getCurHandler();
    return p != nullptr ? p->m_scheduler : nullptr;
}

Coroutine * Cohandler::getCurCoroutine()
{
    auto p = getCurHandler();
    if(p != nullptr && p->is_curco_vaild)
    {
        return *p->m_cur_running_co;
    }
    else
        return nullptr;
}

Cohandler::CoIterator Cohandler::getCurCoInterator()
{
    auto p = getCurHandler();
    if(p != nullptr && p->is_curco_vaild)
        return p->m_cur_running_co;
    else
        TOY_ASSERT(false);
    
}

void Cohandler::handler()
{
    getCurHandler() = this; // set this handler to cur thread
    auto p = getCurHandler();
    printf("%d, %d.\n", this, p);
    while(m_scheduler->isRunning())
    {
        if(m_runnable_cos.empty())
        {
            turnNew2Runnable();

            if(m_runnable_cos.empty())
            {
                waitForNewCos();
                turnNew2Runnable();
                continue;
            }

        }
        // get a coroutine to run
        m_cur_running_co = m_runnable_cos.begin();
        is_curco_vaild = true;

        // TODO: is this while-loop necessary
        while(m_cur_running_co != m_runnable_cos.end() 
            && (*m_cur_running_co) != nullptr
            && m_scheduler->isRunning()) // keeping running
        {
            //TODO: how to get the next co after cur co

            auto cur_co = *m_cur_running_co;
            cur_co->m_state = Coroutine::CoState::RUNNING;
            cur_co->m_cohandler = this;

            cur_co->swapIn();

            switch(cur_co->m_state)
            {
                // TODO
                case Coroutine::CoState::RUNNING : 
                case Coroutine::CoState::NORMAL : 
                {
                    // add it to the end of runnable list
                    m_runnable_cos.splice(m_runnable_cos.end(), m_runnable_cos, m_cur_running_co);
                    // get a new one;
                    m_cur_running_co = m_runnable_cos.begin();
                    is_curco_vaild = true;
                    break;
                }
                case Coroutine::CoState::SUSPEND : 
                    // wake it up after a setting time
                case Coroutine::CoState::DONE : 
                {
                    // delete or reuse it
                    is_curco_vaild = false;
                    m_runnable_cos.erase(m_cur_running_co);
                    m_co_count--;
                    delete cur_co;
                    m_cur_running_co = m_runnable_cos.begin();
                    is_curco_vaild = true;
                    printf("Co Done.\n");

                }

            }

        }
        is_curco_vaild = false;

        
    }
}

void Cohandler::yeild()
{
    //Coroutine * cur_co = getCurCoroutine();
    //TOY_ASSERT(cur_co != nullptr);
    //cur_co->swapOut();

    auto it = getCurCoInterator();
    (*it)->swapOut();
    
}

void Cohandler::suspend()
{
    auto handler = getCurHandler();
    if(handler != nullptr)
        handler->suspendCo(); // TODO
}

void Cohandler::turnNew2Runnable()
{
    m_runnable_cos.splice(m_runnable_cos.end(), m_new_cos);
}

void Cohandler::addCoroutine(Coroutine * co)
{
    std::unique_lock<std::mutex> lock(m_cos_mutex);
    m_new_cos.push_back(co);
    m_co_count++;
    co->m_cohandler = this;
    if(is_waitting)
    {
        m_sema.notify();
    }
}
void Cohandler::waitForNewCos()
{
    m_sema.wait();
    is_waitting = false;
}

void Cohandler::yeildCo()
{

}

void Cohandler::suspendCo()
{
    // Coroutine * cur_co = getCurCoroutine();
    // TOY_ASSERT(cur_co != nullptr);
    // TOY_ASSERT(cur_co->m_cohandler != nullptr);
    // TOY_ASSERT(cur_co->m_state == Coroutine::CoState::RUNNING);
    auto p = getCurCoInterator();

    (*p)->m_state = Coroutine::CoState::SUSPEND;
    std::unique_lock<std::mutex> lock(m_cos_mutex); // m_cos_mutex can not be used in static function
    //TODO: erase cur_co from runnable list
    //      add it to waitting list;
    m_waiting_cos.splice(m_waiting_cos.end(), m_runnable_cos, p);
}

} // namespace Toy