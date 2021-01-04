#include "IOManager.h"
#include <sys/epoll.h>
#include <unistd.h> // for close
#include "Util.h"
#include "Log.h"
#include "ToyCo.h"

namespace Toy
{


IOManager::FdContext::EventContext & IOManager::FdContext::getEventContext(Event event)
{
    switch (event)
    {
        case IOManager::Event::READ:
            return read_ectx;
        case IOManager::Event::WRITE:
            return write_ectx;
        default:
            TOY_ASSERT(false);
    }
}

void IOManager::FdContext::triggerEvent(Event event)
{
    TOY_ASSERT(events & event);
    events = static_cast<Event>(events & ~event);
    EventContext & ctx = getEventContext(event);
    if(ctx.cb)
    {
        TOY_CO_CREATE(ctx.cb);
    }
}

IOManager::IOManager()
{
    m_epoll_fd = epoll_create1(0);
    TOY_ASSERT(m_epoll_fd > 0);

    //epoll_event
}

IOManager::~IOManager()
{

}

void IOManager::closeAll()
{
    stop();
    close(m_epoll_fd);
    std::unique_lock<MutexType> lock(m_mutex);
    for(auto & p : m_all_fdctxs)
        delete p.second;
    m_all_fdctxs.clear();

}

int IOManager::addEvent(int fd, Event event, CallBackFun cb)
{
    // 获取对应的 FdContext
    FdContext * fd_ctx = nullptr;
    {
        std::unique_lock<MutexType> lock(m_mutex);
        if(m_all_fdctxs.find(fd) != m_all_fdctxs.end())
        {
            fd_ctx = m_all_fdctxs[fd];
        }
        else
        {
            fd_ctx = new FdContext();
            m_all_fdctxs[fd] = fd_ctx;
        }
    }

    std::unique_lock<MutexType> lock(fd_ctx->mutex);
    // 如果当前fd对应的FdContext已将加过该事件了
    if(fd_ctx->events & event)
    {
        // 当出现这种情况时，是否是异常。
        // 可能有些事件的上下文正在执行，此时会造成异常？？
        // if(event & IOManager::Event::READ)
        // {
        //     fd_ctx->read_ectx.cb = cb;
        // }
        // else if(event & IOManager::Event::WRITE)
        // {
        //     fd_ctx->write_ectx.cb = cb;
        // }
        // else
        // {
        //     return -1;
        // }
        //TOY_ASSERT(!(fd_ctx->events & event));
        return -1;
    }

    int op = fd_ctx->events ? EPOLL_CTL_MOD : EPOLL_CTL_ADD;
    epoll_event epollevent;
    epollevent.events = EPOLLET | fd_ctx->events | event;
    epollevent.data.ptr = fd_ctx;

    int ret = epoll_ctl(m_epoll_fd, op, fd, &epollevent);
    if(ret != 0)
    {
        TOY_LOG_DEBUG << "epoll_ctl("<< m_epoll_fd << ", "
            << op << ", " << fd << ", " << (EPOLL_EVENTS)epollevent.events << "):"
            << ret << " (" << errno << ") (" << strerror(errno) << ") fd_ctx->events = "
            << (EPOLL_EVENTS)fd_ctx->events;
        return -1;
    }

    fd_ctx->events = static_cast<Event>(fd_ctx->events | event);
    FdContext::EventContext & ectx = fd_ctx->getEventContext(event);

    // 在当前添加新事件场合下，要保证对应事件的ectx是未经过设定的
    TOY_ASSERT(!ectx.ctx && !ectx.cb);

    // TODO : 如何处理 ectx.ctx 的创建
   
    return -1; //TODO
}

bool IOManager::delEvent(int fd, Event event)
{
    FdContext * fd_ctx = nullptr;
    {
        std::unique_lock<MutexType> lock(m_mutex);
        if(m_all_fdctxs.find(fd) != m_all_fdctxs.end())
        {
            fd_ctx = m_all_fdctxs[fd];
        }
        else
            return false;
    }
    
    std::unique_lock<MutexType> lock(fd_ctx->mutex);
    if(!(fd_ctx->events & event))
    {
        return false;
    } 
    Event new_event = static_cast<Event>(fd_ctx->events & ~event);
    //int op = new_events != Event::NONE ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
    int ret = 0;
    if(new_event == Event::NONE)
    {
        ret = epoll_ctl(m_epoll_fd, EPOLL_CTL_DEL, fd, nullptr);
    }
    else
    {
        epoll_event epollevent;
        epollevent.events = EPOLLET | new_event;
        epollevent.data.ptr = fd_ctx;
        ret = epoll_ctl(m_epoll_fd, EPOLL_CTL_MOD, fd, &epollevent);
    }
    if(ret != 0)
    {
        TOY_LOG_DEBUG << "epoll_ctl error in IOManager::delEvent : fd = " << fd
            << ", Events" << new_event;
        return false; 
    }
    
    fd_ctx->events = new_event;
    // TODO : reset fd_ctx->read or write ctx
    
    return false; //TODO
    
}

bool IOManager::finishEvent(int fd, Event event)
{
    FdContext * fd_ctx = nullptr;
    {
        std::unique_lock<MutexType> lock(m_mutex);
        if(m_all_fdctxs.find(fd) != m_all_fdctxs.end())
        {
            fd_ctx = m_all_fdctxs[fd];
        }
        else
            return false;
    }
    std::unique_lock<MutexType> lock(fd_ctx->mutex);
    Event new_event = static_cast<Event>(fd_ctx->events & ~event);
    //int op = new_events != Event::NONE ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
    int ret = 0;
    if(new_event == Event::NONE)
    {
        ret = epoll_ctl(m_epoll_fd, EPOLL_CTL_DEL, fd, nullptr);
    }
    else
    {
        epoll_event epollevent;
        epollevent.events = EPOLLET | new_event;
        epollevent.data.ptr = fd_ctx;
        ret = epoll_ctl(m_epoll_fd, EPOLL_CTL_MOD, fd, &epollevent);
    }
    if(ret != 0)
    {
        TOY_LOG_DEBUG << "epoll_ctl error in IOManager::finishEvent : fd = " << fd
            << ", Events" << new_event;
        return false; 
    }
    fd_ctx->triggerEvent(event);
    return true;
}

bool IOManager::finishAll(int fd)
{
    FdContext * fd_ctx = nullptr;
    {
        std::unique_lock<MutexType> lock(m_mutex);
        if(m_all_fdctxs.find(fd) != m_all_fdctxs.end())
        {
            fd_ctx = m_all_fdctxs[fd];
        }
        else
            return false;
    }
    std::unique_lock<MutexType> lock(fd_ctx->mutex);
   
    int ret = 0;

    ret = epoll_ctl(m_epoll_fd, EPOLL_CTL_DEL, fd, nullptr);
    if(ret != 0)
    {
        TOY_LOG_DEBUG << "epoll_ctl error in IOManager::finishAll : fd = " << fd;
        return false; 
    }
    if(fd_ctx->events & Event::READ)
        fd_ctx->triggerEvent(Event::READ);
    if(fd_ctx->events & Event::WRITE)
        fd_ctx->triggerEvent(Event::WRITE);

    TOY_ASSERT(fd_ctx->events == Event::NONE);
    return true;

}

// void IOManager::setFdctxsCount(size_t count)
// {
// }

} // namespace Toy