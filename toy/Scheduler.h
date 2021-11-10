#pragma once
//#include <vector>
#include <list>
#include <thread>
#include <atomic>
#include <mutex>
#include <functional>
#include <memory>

#include "Noncopyable.h"
#include "Coroutine.h"
#include "Cohandler.h"
#include "Timer.h"

namespace Toy
{

class Cohandler;
class Coroutine;
typedef std::function<void ()> CoFunction;

class Scheduler : Noncopyable
{
public:
    typedef std::shared_ptr<TimerWheel> TimerPtr;
    Scheduler();
    ~Scheduler();
    
    void createCoroutine(const CoFunction & cf, size_t stack_size = DEFAULT_STACK_SIZE);
    void start(size_t thread_num = 0, bool using_cur_thread = true);
    void stop();
    void stopGently(); // TODO
    void stop(uint64_t timeout); // TODO
    size_t getCoNum();
    inline bool isRunning() { return is_running; }
    TimerPtr getTimer() { return m_timer; }
private:
    void dispatch();
    void addCoroutine(Coroutine * co);

    std::list<Cohandler *> m_all_handlers;
    std::mutex m_handlers_mutex;


    size_t m_running_id;
    size_t m_stack_size;

    std::atomic<bool> is_running = {false};

    TimerPtr m_timer;

    // TODO: Exit List 
};




} // namespace Toy