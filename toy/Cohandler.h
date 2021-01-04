#pragma once
#include "Noncopyable.h"
#include "Coroutine.h"
#include "Scheduler.h"
#include "Semaphore.h"
#include "Timer.h"
//#include "Queue.h"

#include <list>
#include <mutex>
#include <atomic>
#include <unordered_set>

namespace Toy
{

class Scheduler;
class Coroutine;
class Cohandler : public Noncopyable
{
    friend class Scheduler;
public:
    struct SuspendInfo
    {
        //typedef std::weak_ptr<Coroutine>  WatchPtr;
        typedef Coroutine * WatchPtr;
        WatchPtr sus_co; // TODO： 有用吗？ 还是说全部换成智能指针
    };  
public:
    //typedef Queue<Coroutine> CoQueue;
    typedef std::list<Coroutine *> CoQueue;
    typedef std::list<Coroutine *>::iterator CoIterator;
    typedef std::mutex LockType;
    //typedef std::chrono::duration<std::chrono::milliseconds> TimeDuration

    Cohandler(Scheduler * sd, int id);
    ~Cohandler();
  
    // 提供给用户使用，用户不用关注当前是哪个Cohandler
    static void yeild();
    static SuspendInfo suspend(TimerWheel::TimeDuration dur);
    //TODO
    //static void suspend(Time);
    static bool wakeup(SuspendInfo si);

private:
    void handler(); // 不提供给外部使用
    // 获取当前线程的handler
    static Cohandler * & getCurHandler();
    static Scheduler * getCurScheduler();
    static Coroutine * getCurCoroutine();
    //static CoIterator getCurCoIterator();

    // 线程安全
    void turnNew2Runnable();
    // 线程安全
    void addCoroutine(Coroutine * co);
    void waitForNewCos();

    void yeildCo();
    SuspendInfo suspendCo();
    bool wakeupCo(SuspendInfo);

    std::atomic<bool> is_running;
    std::atomic<bool> is_waitting;
    std::atomic<bool> is_active;

    std::atomic<uint64_t> m_co_count;

    Semaphore m_sema; // 等待新协程的信号

    CoQueue m_runnable_cos;
    CoQueue m_new_cos;
    std::unordered_set<Coroutine *> m_waiting_cos;

    LockType m_run_lock;
    LockType m_wait_lock;
    LockType m_new_lock;

    LockType m_cos_mutex;

    Coroutine * m_cur_running_co;
    //Coroutine * m_next_co;
    //CoIterator m_cur_running_co;
    //std::atomic<bool> is_curco_vaild = {false};

    Scheduler * m_scheduler;
    int m_id;
};

} // namespace Toy