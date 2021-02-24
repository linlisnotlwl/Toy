#include "ReactorElement.h"
#include "Reactor.h"
#include <poll.h>
#include <sys/epoll.h>

namespace Toy
{

uint32_t turnIOEvent2EpollEvent(IOEvent io_event)
{
    uint32_t epoll_event = 0;
    if(io_event & IOEvent::IN)
        epoll_event |= EPOLLIN;
    if(io_event & IOEvent::OUT)
        epoll_event |= EPOLLOUT;
    if(io_event & (IOEvent::ERROR | IOEvent::NVAL))
        epoll_event |= EPOLLERR;
    if(io_event & IOEvent::HUP)
        epoll_event |= EPOLLHUP;
    if(io_event & IOEvent::PRI)
        epoll_event |= EPOLLPRI;
    return epoll_event;    
}

short turnIOEvent2PollEvent(IOEvent io_event)
{
    short poll_event = 0;
    if(io_event & IOEvent::IN)
        poll_event |= POLLIN;
    if(io_event & IOEvent::OUT)
        poll_event |= POLLOUT;
    if(io_event & IOEvent::ERROR)
        poll_event |= POLLERR;
    if(io_event & IOEvent::HUP)
        poll_event |= POLLHUP;
    if(io_event & IOEvent::NVAL)
        poll_event |= POLLNVAL;
    if(io_event & IOEvent::PRI)
        poll_event |= POLLPRI;
    return poll_event;
}

IOEvent turnEpollEvent2IOEvent(uint32_t epoll_event)
{
    IOEvent io_event = IOEvent::NONE;
    if(epoll_event & EPOLLIN)
        io_event = static_cast<IOEvent>(io_event |  IOEvent::IN);
    if(io_event & EPOLLOUT)
        io_event = static_cast<IOEvent>(io_event |  IOEvent::OUT);
    if(epoll_event & EPOLLERR)
        io_event = static_cast<IOEvent>(io_event |  IOEvent::ERROR);
    if(epoll_event & EPOLLHUP)
        io_event = static_cast<IOEvent>(io_event |  IOEvent::HUP);
    if(epoll_event & POLLPRI)
        io_event = static_cast<IOEvent>(io_event |  IOEvent::PRI);
    return io_event;
}

IOEvent turnPollEvent2IOEvent(short poll_event)
{
    IOEvent io_event = IOEvent::NONE;
    if(poll_event & (POLLIN | POLLRDBAND | POLLRDNORM))
        io_event = static_cast<IOEvent>(io_event | IOEvent::IN);
    if(io_event & (POLLOUT | POLLWRBAND | POLLWRNORM))
        io_event = static_cast<IOEvent>(io_event | IOEvent::OUT);
    if(poll_event & POLLERR)
        io_event = static_cast<IOEvent>(io_event | IOEvent::ERROR);
    if(poll_event & POLLHUP)
        io_event = static_cast<IOEvent>(io_event | IOEvent::HUP);
    if(poll_event & POLLNVAL)
        io_event = static_cast<IOEvent>(io_event | IOEvent::NVAL);
    if(poll_event & POLLPRI)
        io_event = static_cast<IOEvent>(io_event | IOEvent::PRI);
    return io_event;
}

ReactorElement::ReactorElement(int fd) : m_fd(fd)
{
    // TODO

}

bool ReactorElement::add(Reactor * reactor, IOEvent io_event, const Entry & entry)
{ 
    // 新添加事件
    IOEvent new_add_event = static_cast<IOEvent>(io_event & (IOEvent::IN | IOEvent::OUT));
    // 如果是其他event则设置为POLLERR
    if(new_add_event == 0)
        new_add_event = IOEvent::ERROR;
    
    std::unique_lock<std::mutex> lock(m_mutex);

    // 期望发生的总的event
    IOEvent promise_event = static_cast<IOEvent>( m_event | new_add_event);
    new_add_event = static_cast<IOEvent>(promise_event & ~m_event);

    if(promise_event != m_event)
    {
        if(!reactor->addEvent(m_fd, new_add_event, promise_event))
        {
            return false;
        }
        m_event = promise_event;
        addEntryWithoutLock(new_add_event, entry);
    }

    return true;
}

void ReactorElement::trigger(Reactor * reactor, IOEvent trigger_event)
{
    // 需要检测的错误事件
    IOEvent error_event = static_cast<IOEvent>(IOEvent::HUP | IOEvent::NVAL);
    // 剩下的期待发生的总事件
    IOEvent promise_event = IOEvent::NONE;
    std::unique_lock<std::mutex> lock(m_mutex);
    // 根据事件调用triggerEntryList
    // 当出现error_event时，有可能是其中任意的事件导致的
    if(trigger_event & (error_event | IOEvent::IN))
    {
        triggerEntryListWithoutLock(m_in_elist);
    }
    if(trigger_event & (error_event | IOEvent::OUT))
    {
        triggerEntryListWithoutLock(m_out_elist);
    }
    if(trigger_event & (error_event | IOEvent::ERROR))
    {
        triggerEntryListWithoutLock(m_error_elist);
    }
    if(trigger_event & (error_event | IOEvent::IN | IOEvent::OUT))
    {
        triggerEntryListWithoutLock(m_in_and_out_elist);
    }
    
    if(!m_in_elist.empty())
        promise_event = static_cast<IOEvent>(promise_event | IOEvent::IN);
    if(!m_out_elist.empty())
        promise_event = static_cast<IOEvent>(promise_event | IOEvent::OUT);
    if(!m_in_and_out_elist.empty())
        promise_event = static_cast<IOEvent>(promise_event | IOEvent::IN | IOEvent::OUT);
    if(!m_error_elist.empty())
        promise_event = static_cast<IOEvent>(promise_event | IOEvent::ERROR);
    
    IOEvent del_event = static_cast<IOEvent>(m_event & ~promise_event);

    if(promise_event != m_event)
    {
        if(reactor && reactor->delEvent(m_fd, del_event, promise_event))
            m_event = promise_event;
    } 

}

void ReactorElement::triggerEntryListWithoutLock(EntryList & entry_list)
{
    for(const auto & entry : entry_list)
    {
        Cohandler::wakeup(entry.si);
    }
    entry_list.clear();
}

void ReactorElement::addEntryWithoutLock(IOEvent new_add_event, const Entry & entry)
{
    if((new_add_event & IOEvent::IN) && (new_add_event & IOEvent::OUT))
        m_in_and_out_elist.push_back(entry);
    else if(new_add_event & IOEvent::IN)
        m_in_elist.push_back(entry);
    else if(new_add_event & IOEvent::OUT)
        m_out_elist.push_back(entry);
    else
        m_error_elist.push_back(entry);
}

} // namespace Toy