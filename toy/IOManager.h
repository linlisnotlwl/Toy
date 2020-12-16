#pragma once
#include "Scheduler.h"
#include <mutex>
#include <vector>
#include <unordered_map>
#include <functional>


// IOManager 用于处理一些（要阻塞的）定时任务

namespace Toy
{

class IOManager : public Scheduler
{

public:
    typedef std::mutex MutexType;
    typedef std::function<void ()> CallBackFun;
    struct FdContext;
    typedef std::unordered_map<int, FdContext *> FdContextSet;
    enum Event
    {
        NONE = 0x0,
        READ = 0x1, //  EPOLL_EVENTS::EPOLLIN
        WRITE = 0x4 //  EPOLL_EVENTS::EPOLLOUT
    };
    IOManager();
    ~IOManager();
    void closeAll();

    int addEvent(int fd, Event event, CallBackFun cb = nullptr);
    // delete specified event of fd
    bool delEvent(int fd, Event event);
    // trigger the event once and then delete it
    bool finishEvent(int fd, Event event);
    bool finishAll(int fd);

    static IOManager * getCurrentIOManager();
private:
    // 每个fd文件处理的context
    struct FdContext
    {
        typedef std::mutex MutexType;
        // 事件的context
        struct EventContext
        {
            Context * ctx;
            CallBackFun cb;
        };
        EventContext & getEventContext(Event event);
        void triggerEvent(Event event);
        EventContext read_ectx;
        EventContext write_ectx;
        int fd = 0;
        Event events = NONE; // all events | together
        MutexType mutex;
    };
private:
    void worker(); // handler epoll events
    //void setFdctxsCount(size_t count); use it while using vector to store factxs

    MutexType m_mutex; // protect m_all_fdctxs
    int m_epoll_fd = 0;
        //std::vector<FdContext *> m_all_fdctxs; // store all fd event ctxs
        // using vector beceuse we assume that most of fds(add in the iomanager) are adjacent
        // but it waste some space
    FdContextSet m_all_fdctxs;  // store all fd event ctxs

};



} // namespace Toy