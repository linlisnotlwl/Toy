#pragma once

#include <mutex>
#include <condition_variable>
#include <chrono>
#include "Noncopyable.h"
namespace Toy
{

class Semaphore : Noncopyable
{
public:
    Semaphore(int count = 0);
    //Semaphore(const Semaphore &) = delete;
    //Semaphore& operator=(const Semaphore &) = delete;
    void notify();
    void wait();
    bool tryWait();
    bool waitFor(const std::chrono::milliseconds &);
    bool waitUntil(const std::chrono::time_point<std::chrono::high_resolution_clock> &);

private:
    std::mutex _mutex;
    std::condition_variable _cv;
    int _count;

};

} // namespace Toy
