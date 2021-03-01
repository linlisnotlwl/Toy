#include "Reactor.h"
#include "FdManager.h"
#include "Util.h" // for CallWithoutINTR
#include "Log.h"
#include <unistd.h> // for close
#include <sys/epoll.h>

namespace Toy
{

std::vector<Reactor *> Reactor::m_all_reactors; //静态变量初始化

Reactor & Reactor::getReactor(int fd)
{
    static int ignore = initReactor(1); // 初始调用一次，也可使用call_once
    (void)ignore;
    return *m_all_reactors[fd % m_all_reactors.size()];
}


int Reactor::initReactor(int num)
{
    if(!m_all_reactors.empty())
        return 0;
    for(int i = 0; i < num; ++i)
        m_all_reactors.push_back(new EpollReactor);
    return 0;
}

Reactor::Reactor()
{

}

bool Reactor::add(int fd, IOEvent io_event, const Entry & entry)
{
    auto fd_ctx = FdMgr::getInstance().getFdCtx(fd);
    if(fd_ctx == nullptr)
        return false;
    return fd_ctx->add(this, io_event, entry);
}

void Reactor::startLoopThread()
{
    std::thread t([this]()
    {  
        this->run(); 
    });
    t.detach();
}

//--------------------------------------------------


EpollReactor::EpollReactor()
{
    // Since Linux 2.6.8, the size argument
    // is ignored, but must be greater than zero.
    m_epoll_fd = epoll_create(1024);
    startLoopThread();
}

EpollReactor::~EpollReactor()
{
    if(m_running)
    {
        m_running = false;
        m_sema.waitFor(std::chrono::milliseconds(10));
    }
    close(m_epoll_fd);
}

bool EpollReactor::addEvent(int fd, IOEvent new_event, IOEvent promise_event) 
{
    epoll_event ep_event;
    ep_event.events = turnIOEvent2EpollEvent(promise_event) | EPOLLET;
    ep_event.data.fd = fd;
    int op = new_event == promise_event ? EPOLL_CTL_ADD : EPOLL_CTL_MOD;
    //是否需要屏蔽中断，为什么? 内部应该处理中断，以提供给外部统一的状态
    //int res = epoll_ctl(m_epoll_fd, op, fd, &ep_event);
    TOY_LOG_DEBUG << "EpollReactor addEvent, op = " << op << "(1:EPOLL_CTL_ADD;2:EPOLL_CTL_MOD)";
    int res = CallWithoutINTR<int>(::epoll_ctl, m_epoll_fd, op, fd, &ep_event);
    
    return res == 0;

}

bool EpollReactor::delEvent(int fd, IOEvent del_event, IOEvent promise_event)
{
    epoll_event ep_event;
    ep_event.events = turnIOEvent2EpollEvent(promise_event) | EPOLLET;
    ep_event.data.fd = fd;
    int op = 0 == promise_event ? EPOLL_CTL_DEL : EPOLL_CTL_MOD;
    //int res = epoll_ctl(m_epoll_fd, op, fd, &ep_event);
    int res = CallWithoutINTR<int>(::epoll_ctl, m_epoll_fd, op, fd, &ep_event);
    return res == 0;
}

void EpollReactor::run()
{
    static const int MAX_EVENT_NUM = 1024;
    static const int WAIT_TIME = 10; // ms
    epoll_event events[MAX_EVENT_NUM];
    m_running = true;
    while(m_running)
    {
        //int res = epoll_wait(m_epoll_fd, events, MAX_EVENT_NUM, WAIT_TIME);
        int res = CallWithoutINTR<int>(::epoll_wait, m_epoll_fd, events, MAX_EVENT_NUM, WAIT_TIME);
        for(int i = 0; i < res; ++i)
        {
            auto fd_ctx = FdMgr::getInstance().getFdCtx(events[i].data.fd);
            if(!fd_ctx)
                continue;            
            TOY_LOG_DEBUG << "epollReactor trigger. fd = " << fd_ctx->getFd() 
                << ", epoll_event = " << events[i].events << "(1:in; 4:out)";
            fd_ctx->trigger(this, turnEpollEvent2IOEvent(events[i].events));

        }
    }
    m_sema.notify();
}

} // namespace Toy