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
    // TODO: 是直接清空吗   要锁吗？？
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
    if(p != nullptr)
    {
        return p->m_cur_running_co;
    }
    else
        return nullptr;
}
/*
Cohandler::CoIterator Cohandler::getCurCoIterator()
{
    auto p = getCurHandler();
    if(p != nullptr && p->is_curco_vaild)
        return p->m_cur_running_co;
    else
        TOY_ASSERT(false);
    
}
*/

void Cohandler::handler()
{
    getCurHandler() = this; // set this handler to cur thread
    //auto p = getCurHandler();
    //printf("%d, %d.\n", this, p);
    while(m_scheduler->isRunning())
    {
        std::unique_lock<LockType> lock1(m_run_lock, std::defer_lock);
        if(m_runnable_cos.empty())
        {
            turnNew2Runnable();
            lock1.lock(); // 确保下面的empty是安全的
            if(m_runnable_cos.empty())
            {
                lock1.unlock();
                waitForNewCos();
                turnNew2Runnable();
                continue;
            }
            else
                lock1.unlock();
        }
        lock1.lock();
        // 直接弹出该协程好像比较好
        // get a coroutine to run
        m_cur_running_co = m_runnable_cos.front();
        m_runnable_cos.pop_front();

        // this while-loop 消耗完runnable的所有协程任务
        while(m_cur_running_co != nullptr
            && m_scheduler->isRunning()) // keeping running
        {
            //auto cur_co = m_cur_running_co;
            m_cur_running_co->m_state = Coroutine::CoState::RUNNING;
            m_cur_running_co->m_cohandler = this; // 这里设置的原因是，它有可能被调度到其他控制器

            lock1.unlock(); // 任务执行中可能出现的suspend，会改变队列
            //printf("come out.\n");
            m_cur_running_co->swapIn();
            //printf("come back.\n");
            lock1.lock();

            switch(m_cur_running_co->m_state)
            {
                case Coroutine::CoState::RUNNING : 
                case Coroutine::CoState::NORMAL : 
                {
                    // add it to the end of runnable list
                    m_runnable_cos.push_back(m_cur_running_co);
                    m_cur_running_co = nullptr;
                    break;
                }
                case Coroutine::CoState::SUSPEND : 
                {
                    // 因为调用suspend的时候会处理？？还是回到这里处理呢 不会
                    m_cur_running_co = nullptr;
                    break;
                }
                case Coroutine::CoState::DONE : 
                {
                    // delete or reuse it 
                    // TODO: Add it to a reuse list: 等待一段时间没有重用就删除
                    m_co_count--;
                    delete m_cur_running_co;
                    m_cur_running_co = nullptr;
                    //printf("Co Done.\n");
                    break;
                }
            }
            // get new one
            if(!m_runnable_cos.empty())
            {
                m_cur_running_co = m_runnable_cos.front();
                m_runnable_cos.pop_front();
            }
        }  
    }
}

void Cohandler::yeild()
{
    Coroutine * cur_co = getCurCoroutine();
    TOY_ASSERT(cur_co != nullptr);
    cur_co->swapOut();

    //auto it = getCurCoIterator();
    //(*it)->swapOut();
    
}

Cohandler::SuspendInfo Cohandler::suspend()
{
    auto handler = getCurHandler();
    TOY_ASSERT(handler != nullptr);
    auto si = handler->suspendCo();
    return si;
}

Cohandler::SuspendInfo Cohandler::suspend(TimerWheel::TimeDuration dur)
{
    auto handler = getCurHandler();
    TOY_ASSERT(handler != nullptr);
    auto si = handler->suspendCo();
    // mutable 默认情况下，对于一个值被拷贝的变量，
    // lambda不会改变其值，如果我们希望能改变一个被捕获变量的值，
    // 就必须在参数列表尾加上关键字mutable。
    getCurScheduler()->getTimer()->add(dur, 
        [si]() 
        {
            Cohandler::wakeup(si);
            // bool state = Cohandler::wakeup(si);
            // if(state)
            //     printf("WakeUP Success !\n");
            // else
            //     printf("WakeUP Fail !\n");
        });
    return si;
}

bool Cohandler::wakeup(SuspendInfo si)
{
    if(si.sus_co == nullptr)
        return false;
    // TODO: 是否需要锁Coroutine
    auto cohandler = si.sus_co->getCohandler();
    return cohandler == nullptr ? false : cohandler->wakeupCo(si);
}

bool Cohandler::wakeupCo(SuspendInfo si)
{
    TOY_ASSERT(si.sus_co != nullptr);
    TOY_ASSERT(si.sus_co->m_state == Coroutine::CoState::SUSPEND);
    std::unique_lock<LockType> lock1(m_wait_lock);
    auto it = m_waiting_cos.find(si.sus_co);
    if(it == m_waiting_cos.end())
    {
        fprintf(stderr, "Waking up a Co which is not in Waitting Queue.\n");
        TOY_ASSERT(false);
        return false;
    }
    m_waiting_cos.erase(it);
    lock1.unlock();
    si.sus_co->m_state = Coroutine::CoState::RUNNING;
    std::unique_lock<LockType> lock2(m_run_lock);
    m_runnable_cos.push_back(si.sus_co);
    lock2.unlock();
    if(is_waitting)
    {
        m_sema.notify();
    }
    return true;
}

void Cohandler::turnNew2Runnable()
{
    std::unique_lock<LockType> lock2(m_new_lock);
    if(m_new_cos.empty())
        return;
    CoQueue temp(std::move(m_new_cos));
    lock2.unlock();

    std::unique_lock<LockType> lock1(m_run_lock);
    m_runnable_cos.splice(m_runnable_cos.end(), temp);
}

void Cohandler::addCoroutine(Coroutine * co)
{
    std::unique_lock<std::mutex> lock(m_new_lock);
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
    is_waitting = true;
    m_sema.wait();
    is_waitting = false;
}

void Cohandler::yeildCo()
{

}

Cohandler::SuspendInfo Cohandler::suspendCo()
{
    Coroutine * cur_co = getCurCoroutine();
    TOY_ASSERT(cur_co != nullptr);
    TOY_ASSERT(cur_co->m_cohandler != nullptr);
    TOY_ASSERT(cur_co->m_state == Coroutine::CoState::RUNNING);
    // TOY_ASSERT(is_curco_vaild);
    // auto p = getCurCoIterator();

    cur_co->m_state = Coroutine::CoState::SUSPEND;
    std::unique_lock<LockType> lock(m_wait_lock); // non-static member var can not be used in static function
    //  add it to waitting list;
    // how to find its location, use hash? or SuspendInfo store the Iterator 
    m_waiting_cos.insert(cur_co); 
    //m_cur_running_co = nullptr; 还得是当前co，为了后面的yeild使用
    SuspendInfo ret;
    ret.sus_co = cur_co;

    return ret;
}

} // namespace Toy