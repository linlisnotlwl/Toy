#include "Coroutine.h"

namespace Toy
{

Coroutine::Coroutine(CoFunction cf, size_t stack_size)
    : m_cf(cf),  m_state(CoState::NORMAL),
    m_context(Coroutine::run, (void *)this, stack_size)
{

}
Coroutine::~Coroutine()
{

}
void Coroutine::swapIn()
{
    m_context.swapIn();
}

void Coroutine::swapOut()
{
    m_context.swapOut();
}

void Coroutine::run(void * cur_co)
{
    Coroutine * p = static_cast<Coroutine *>(cur_co);
    p->m_cf();
    p->m_state = CoState::DONE;
    p->m_cf = Coroutine::emptyFunction;
}

void Coroutine::emptyFunction()
{
    // do nothing
}

Cohandler * Coroutine::getCohandler()
{
    return m_cohandler;
}

} // namespace Toy