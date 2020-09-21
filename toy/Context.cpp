#include "Context.h"
#include <cstring>
#include <mutex> // for call_once
#include "Util.h"

namespace Toy
{
// static void run(intptr_t vp)
// {
//     Context * ctx = static_cast<Context *>(vp);
//     ctx->m_fun();
// }


Context::Context(Function f, void * cur_co, size_t stack_size)
{
    if(stack_size > MAX_STACK_SIZE)
        stack_size = MAX_STACK_SIZE;
    else if(stack_size < DEFAULT_STACK_SIZE)
        stack_size = DEFAULT_STACK_SIZE;

    m_ctx = static_cast<ContextType *>(malloc(sizeof(ContextType)));
    memset(m_ctx, 0, sizeof(ContextType));
    TOY_ASSERT(m_ctx != nullptr);
    // init context stack
    m_ctx->uc_stack.ss_sp = malloc(stack_size);
    TOY_ASSERT(m_ctx->uc_stack.ss_sp != nullptr);
    memset(m_ctx->uc_stack.ss_sp, 0, sizeof(ContextType));
    
    m_ctx->uc_stack.ss_size = stack_size;

    m_ctx->uc_link = &getTLSContext();  // 重要，当任务结束时回到主context处理
    if(getcontext(m_ctx))
    {
        TOY_ASSERT(false);
    }
    makecontext(m_ctx, (void (*)())f, 1, cur_co);
}

Context::~Context()
{
    if(m_ctx != nullptr)
    {
        if(m_ctx->uc_stack.ss_sp != nullptr)
            free(m_ctx->uc_stack.ss_sp);
        free(m_ctx);
    }
}

void Context::swapIn()
{
    // 从Cohandler::handler中swapIn时，会把现场保存到TLSContext中，
    // 然后恢复当前协程的现场
    if(swapcontext(&getTLSContext(), m_ctx))
    {
        TOY_ASSERT(false);
    }
    
}

void Context::swapTo(ContextType & ctx)
{
    if(swapcontext(m_ctx, &ctx))
    {
        TOY_ASSERT(false);
    }
}

void Context::swapOut()
{
    if(swapcontext(m_ctx, &getTLSContext()))
    {
        TOY_ASSERT(false);
    }
}

Context::ContextType & Context::getTLSContext()
{
    static thread_local ContextType tls_context;
    static thread_local char stack[DEFAULT_STACK_SIZE];
    static thread_local std::once_flag of;
    std::call_once(of, [&](){
        tls_context.uc_stack.ss_sp = stack;
        tls_context.uc_stack.ss_size = DEFAULT_STACK_SIZE;
    });
    return tls_context;
}



} // namespace Toy