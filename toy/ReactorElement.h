#pragma once
// #include "Reactor.h" 循环引用了
#include "Cohandler.h"
//#include <vector>
#include <list>
namespace Toy
{

enum IOEvent : uint32_t
{
    NONE = 0,
    IN = 0x01, // 数据可读，包括普通数据，优先带数据
    OUT = 0x02, // 数据可写，包括普通数据，优先带数据
    ERROR = 0x04, // 错误(只能用作返回的事件)
    HUP = 0x08, // 挂起(只能用作返回的事件)，写端关闭
    NVAL = 0x10, // 文件描述符未打开(只能用作返回的事件)
    PRI = 0x20, // 高优先级数据可读：// 1、TCP带外数据； 2、3、看手册
    TIMEOUT = 0x40
};

uint32_t turnIOEvent2EpollEvent(IOEvent io_event);
short turnIOEvent2PollEvent(IOEvent io_event);
IOEvent turnEpollEvent2IOEvent(uint32_t epoll_event);
IOEvent turnPollEvent2IOEvent(short poll_event);



class Reactor;
class ReactorElement
{
public:
    struct Entry
    {
        Entry() {}
        Entry(Cohandler::SuspendInfo::Ptr _si) : si(_si) {}
        Cohandler::SuspendInfo::Ptr si = nullptr;
    };
    typedef std::list<Entry> EntryList;
public:
    explicit ReactorElement(int fd);

    // 不同的事件可以被添加到任意的Reactor
    bool add(Reactor * reactor, IOEvent io_event, const Entry & entry);

    // 暂时无法取消，因为不知道添加到那个Reactor，也许可以将信息添加到Entry中
    // bool del();

    // 触发对应事件，唤醒实体
    void trigger(Reactor * reactor, IOEvent trigger_event);

    int getFd() const { return m_fd; };
protected:
    void triggerEntryListWithoutLock(EntryList & entry_list);
    void addEntryWithoutLock(IOEvent new_add_event, const Entry & entry);
    int m_fd;
private:
    std::mutex m_mutex;
    IOEvent m_event;

    EntryList m_in_elist;
    EntryList m_out_elist;
    EntryList m_in_and_out_elist;
    EntryList m_error_elist;

};
} // namespace Toy