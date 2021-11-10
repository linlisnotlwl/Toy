#pragma once

#include <functional>
#include <cstring>
#include "Noncopyable.h"


namespace Toy
{
template<size_t SIZE>
class FixBuffer : Noncopyable
{
public:
    typedef std::function<void()> CallBack;
    FixBuffer() : m_cur_p(m_data)
    {
        setZero();
    }
    void append(const char * data, size_t len)
    {
        if(len < getFreeSize())
        {
            memcpy(m_cur_p, data, len);
            m_cur_p += len;
        }
        else if(len == getFreeSize())
        {
            memcpy(m_cur_p, data, len);
            m_cur_p += len;
            m_callback();
        }
        else
        {
            size_t temp = len - getFreeSize();
            memcpy(m_cur_p, data, temp);
            m_cur_p += temp;
            //TODO : callback to clean data buffer
            //if(m_callback != nullptr)
                //m_callback();
            append(data + temp, len - temp);
        }
    }
    char * getCurrentPos() { return m_cur_p; }
    char * getData() { return m_data; }
    size_t getFreeSize() const { return  sizeof(m_data) - getDataSize(); }
    size_t getDataSize() const { return m_cur_p - m_data; }
    void clear() 
    { 
        m_cur_p = m_data; 
        setZero(); 
    }
    void setZero() { memset(m_data, 0, SIZE); }
    bool empty() const { return m_data == m_cur_p; }
    template<size_t S>
    friend void swap(FixBuffer<S> & a, FixBuffer<S> & b);
private:
    char m_data[SIZE];
    char * m_cur_p = nullptr;
    CallBack m_callback = nullptr;

}; // class FixBuffer

template<size_t SIZE>
void swap(FixBuffer<SIZE> & a, FixBuffer<SIZE> & b)
{
    using std::swap;
    swap(a.m_data, b.m_data);
    swap(a.m_cur_p, b.m_cur_p);
    swap(a.m_callback, b.m_callback);
}
} // namespace Toy