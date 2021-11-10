#pragma once

#include <functional>
#include "Context.h"
//#include <vector>

#include "Noncopyable.h"
#include "Cohandler.h"
//#include "Queue.h" // for LinkBase

namespace Toy
{

typedef std::function<void ()> CoFunction;
class Cohandler;

class Coroutine : Noncopyable//, public LinkBase
{
    friend class Cohandler;
public:
    
    enum CoState
    {
        SUSPEND,    // use it while current context is going to be block;
        RUNNING,    // cur coroutine is running
        NORMAL,     // can be run
        DONE        // job finish, will be deleted or reused?
    };

    Coroutine(CoFunction cb, size_t stack_size = 0);
    ~Coroutine();
    void reset(CoFunction cb);
    bool resume();
    bool yield(CoState state);
    void swapIn();
    void swapOut();
    Cohandler * getCohandler();

private:
    static void run(void * cur_co);
    static void emptyFunction();
    CoFunction m_cf;
    //char * m_stack;
    //size_t m_stack_size;
    CoState m_state;
    Context m_context;
    Cohandler * m_cohandler = nullptr;

};



} // namespace Toy