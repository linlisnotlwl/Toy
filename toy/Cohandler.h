#pragma once
#include "Noncopyable.h"
#include "Coroutine.h"
#include "Scheduler.h"
#include "Semaphore.h"
#include <list>
#include <mutex>
#include <atomic>

namespace Toy
{

class Scheduler;
class Coroutine;
class Cohandler : public Noncopyable
{
    friend class Scheduler;
public:
    typedef std::list<Coroutine *>::iterator CoIterator;

    Cohandler(Scheduler * sd, int id);
    ~Cohandler();
    void handler();
    // 提供给用户使用，用户不用关注当前是哪个Cohandler
    static void yeild();
    static void suspend();
private:

    // 获取当前线程的handler
    static Cohandler * & getCurHandler();
    static Scheduler * getCurScheduler();
    static Coroutine * getCurCoroutine();
    static CoIterator getCurCoInterator();

    void turnNew2Runnable();
    void addCoroutine(Coroutine * co);
    void waitForNewCos();

    void yeildCo();
    void suspendCo();

    std::atomic<bool> is_running;
    std::atomic<bool> is_waitting;
    std::atomic<bool> is_active;

    Semaphore m_sema;

    std::list<Coroutine *> m_runnable_cos;
    std::list<Coroutine *> m_waiting_cos;
    std::list<Coroutine *> m_new_cos;

    std::mutex m_cos_mutex;

    //Coroutine * m_cur_running_co;
    //Coroutine * m_next_co;
    CoIterator m_cur_running_co;
    bool is_curco_vaild;

    Scheduler * m_scheduler;
    int m_id;
};

} // namespace Toy