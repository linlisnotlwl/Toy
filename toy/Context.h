#pragma once
#include <ucontext.h>
#include <functional>
#include "Noncopyable.h"
namespace Toy
{
static const size_t MAX_STACK_SIZE =  1024 * 1024; //bytes
static const size_t DEFAULT_STACK_SIZE = 128 * 1024;

class Context : Noncopyable
{
public:
    typedef ucontext_t ContextType;
    //typedef std::function<void ()> Function;
    typedef  void (*Function)(void *);

    Context(Function f, void * cur_co, size_t stack_size);
    ~Context();
    void swapIn();//inline 
    void swapTo(ContextType & ctx);//inline
    void swapOut();//inline 
    size_t getStackSize() { return m_ctx != nullptr ? m_ctx->uc_stack.ss_size : 0; }
    void resetStack(size_t stack_size);
    ContextType & getTLSContext();
    
    //Function m_fun;
private:

    ContextType * m_ctx;
    
    //char * m_stack = nullptr;
    //size_t m_stack_size = 0;

};
} // namespace Toy