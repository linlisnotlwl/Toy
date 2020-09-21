#pragma once
//#include <vector>
#include <list>
#include <thread>
#include <atomic>
#include <mutex>
#include <functional>

#include "Noncopyable.h"
#include "Coroutine.h"
#include "Cohandler.h"

namespace Toy
{

class Cohandler;
class Coroutine;
typedef std::function<void ()> CoFunction;

class Scheduler : public Noncopyable
{
public:
    Scheduler();
    ~Scheduler();
    
    void createCoroutine(const CoFunction & cf, size_t stack_size = DEFAULT_STACK_SIZE);
    void start(size_t thread_num = 0);
    void stop();
    size_t getCoNum();
    bool isRunning() { return is_running; }

private:
    void dispatch();
    void addCoroutine(Coroutine * co);

    std::list<Cohandler *> m_all_handlers;
    std::mutex m_handlers_mutex;


    size_t m_running_id;
    size_t m_stack_size;

    std::atomic<bool> is_running = {false};

    
};




} // namespace Toy