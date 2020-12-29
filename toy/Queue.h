#pragma once
#include "Util.h" // for TOY_ASSERT
#include "Noncopyable.h"

namespace Toy
{

// 侵入式链接基类
struct LinkBase
{
    inline void linkPrev(LinkBase * _prev);
    inline void unlinkPrev(LinkBase * _prev);

    // 链接，并不知道链接的当个对象还是一串链表
    inline void linkNext(LinkBase * _next)
    {
        TOY_ASSERT(next == nullptr);
        TOY_ASSERT(_next->prev == nullptr);
        next = _next;
        _next->prev = this;

    }
    inline void unlinkNext(LinkBase * _next)
    {
        // TODO: _next的意义何在？？
        TOY_ASSERT(next == _next);
        TOY_ASSERT(_next->prev == this);
        next = nullptr;
        _next->prev = nullptr;
    }
    LinkBase * prev = nullptr;
    LinkBase * next = nullptr;
};

template<typename T>
class List : public Noncopyable
{
    static_assert((std::is_base_of<LinkBase, T>::value), "T must be base of LinkBase.");
public:
    List() : head(nullptr), tail(nullptr), count(0) {}
    List(LinkBase * _head, LinkBase * tail, size_t _count) : 
        head(_head), tail(_tail), count(_count) {}
    List(List<T> && _list) 
    { 
        head = _list.head;
        tail = _list.tail;
        count = _list.count;
        _list.selfEmpty();
    }
    List<T> & operator=(List<T> && _list)
    {
        clear();
        head = _list.head;
        tail = _list.tail;
        count = _list.count;
        _list.selfEmpty();
        return *this;
    }
    ~List()
    {
        //clear();
    }
    LinkBase * getHead() { return head; }
    LinkBase * getTail() { return tail; }
    size_t getSize() const { return count; }
    bool empty() const { return count == 0; }

    // 将其他list移动拼接到当前list后面
    void append(List<T> && _list)
    {
        if(this == &_list || _list.getSize() == 0)
            return;
        if(getSize() == 0)
        {
            *this = std::move(_list);
            return;
        }
        tail->next = _list.head;
        _list.head->prev = tail;
        tail = _list.tail;
        count += _list.count;
        _list.selfEmpty();
    }

    List<T> cut(size_t num)
    {

    }
    bool erase(T * cur)
    {
        if(cur->prev)
            cur->prev->next = cur->next;
        else
            head = static_cast<T *>(cur->next);

        if(cur->next)
            cur->next->prev = cur->prev;
        else
            tail = static_cast<T *>(cur->prev);
        cur->prev = cur->next = nullptr;
        --count;      
    }
    
    // 解除链接中每个元素的关系，清空头、尾和大小
    void clear()
    {
        T * cur = head;
        for(int i = 0; i < count; ++i)
        {
            T * temp = cur->next;
            cur->prev = nullptr;
            cur->next = nullptr;
            cur = temp;
        }
        selfEmpty();
    }
    void selfEmpty()
    {
        head = tail = nullptr;
        count = 0;
    }
private:

    T * head;
    T * tail;
    size_t count;

};


class FakeLock
{
public:
    //FakeLock(){}
    void lock(){}
    void unlock(){}
};


class FakeLockGuard
{
public:
    template<typename T>
    FakeLockGuard(T & lock) {}
};

#define CHOSE(ThreadSafe, Function) if(ThreadSafe) { } else

/**
 * @brief 侵入式队列，实现随机删除，默认是线程安全模式
 * 
 * @tparam T 存储的数据类型，必须基于QueueHook类
 * @tparam ThreadSafe 是否线程安全
 */
template <typename T, bool ThreadSafe = true>
class Queue
{
    // 保证T是基于QueueHook的
    static_assert((std::is_base_of<LinkBase, T>::value), "T must be base of LinkBase.");
    typedef std::mutex RealLock;
    typedef std::unique_lock<RealLock> RealLockGuard;
public:
     
    typedef typename std::conditional<ThreadSafe, RealLock, FakeLock>::type LockType;
    typedef typename std::conditional<ThreadSafe, RealLockGuard, FakeLockGuard>::type LockGuardType

    Queue()
    {
        if(ThreadSafe)
        {

        }
        else
        {
            
        }
        
    }
    ~Queue()
    {

    }
    inline bool empty() 
    {
        LockGuard lockguard(lock);
        return emptyUnsafe();
    }
    inline T * pop()
    {

    }
    inline void push(T * input)
    {

    }
    inline void push(List<T> && elements)
    {
        if(elements.empty())
            return;
        LockGuardType lockguard(lock);
        pushWithoutLock(elements);
    }
    inline bool erase()
    {

    }
private:
    inline T * popWithoutLock()
    {

    }
    inline void pushWithoutLock(List<T> && elements)
    {
        if(elements.empty())
            return;
        TOY_ASSERT(elements.getHead() == nullptr);
        TOY_ASSERT(elements.getTail() == nullptr);
        tail = elements.getHead();
        elements.getHead()->prev = tail;
        tail = elements.getTail();
        count += elements.getSize();
        elements.selfEmpty();
    }
    inline bool eraseWithoutLock()
    {

    }
    inline bool emptyUnsafe()
    {
        return count == 0;
    }
private:
    LockType lock;
    LinkBase * head;
    LinkBase * tail;
    size_t count;
};


} // namespace Toy
