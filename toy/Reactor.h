#pragma once
#include "ReactorElement.h"
#include "Semaphore.h"
#include <vector>
#include <condition_variable>
#include <mutex>

namespace Toy
{


// 分发事件、添加和删除对应fd的事件
class Reactor
{
public:
    static Reactor & getReactor(int fd);
	static int initReactor(int num);
	typedef ReactorElement::Entry Entry;
public:
    Reactor();
	bool add(int fd, IOEvent io_event, const Entry & entry);
	virtual void run() = 0;

    // 由ReactorElement调用
    virtual bool addEvent(int fd, IOEvent new_event, IOEvent promise_event) = 0;
    virtual bool delEvent(int fd, IOEvent del_event, IOEvent promise_event) = 0;
	// 注意：要么实现虚函数，要么定义成纯虚函数，否则会出现 undefined reference to `vtable


protected:
	void startLoopThread();

private:
	static std::vector<Reactor *> m_all_reactors;
};

class EpollReactor : public Reactor
{	
public:
	EpollReactor();
	~EpollReactor();
	bool addEvent(int fd, IOEvent new_event, IOEvent promise_event) override;
	bool delEvent(int fd, IOEvent del_event, IOEvent promise_event) override;
	void run() override;

private:
	int m_epoll_fd;
	bool m_running = false;
	Semaphore m_sema;
};


} // namespace Toy