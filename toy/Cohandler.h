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
    struct SuspendInfo : public std::enable_shared_from_this<SuspendInfo>
    {
        typedef std::shared_ptr<SuspendInfo> Ptr;
        //typedef std::weak_ptr<Coroutine>  WatchPtr;
        typedef Coroutine * WatchPtr;
        typedef std::mutex MutexType;
        SuspendInfo(WatchPtr _sus_co) : sus_co(_sus_co) {}
        WatchPtr sus_co = nullptr; // TODO： 有用吗？ 还是说全部换成智能指针
        // -1:未唤醒; 1:timer唤醒; 2:事件唤醒
        // wakeup函数不改变wakeup状态，由调用wakeup的使用者设置
        short wakeup_state = -1; 
        MutexType mutex;
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
    static SuspendInfo::Ptr suspend(); // 挂起协程，等待别人唤醒它
    static SuspendInfo::Ptr suspend(TimerWheel::TimeDuration dur); // 挂起协程，等待一段时间后自动唤醒
    static bool wakeup(SuspendInfo::Ptr si);

    static Coroutine * getCurCoroutine();
private:
    void handler(); // 不提供给外部使用

    // 获取当前线程的handler
    static Cohandler * & getCurHandler();
    static Scheduler * getCurScheduler();
    //static CoIterator getCurCoIterator();

    // 线程安全
    void turnNew2Runnable();
    // 线程安全
    void addCoroutine(Coroutine * co);
    void waitForNewCos();

    void yeildCo();
    SuspendInfo::Ptr suspendCo();
    bool wakeupCo(SuspendInfo::Ptr);

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