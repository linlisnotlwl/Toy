#include "Semaphore.h"
namespace Toy
{
Semaphore::Semaphore(int count) : _count(count)
{
}

void Semaphore::notify()
{
    std::unique_lock<std::mutex> lock(_mutex);
    ++_count;
    _cv.notify_one();
}

void Semaphore::wait()
{
    std::unique_lock<std::mutex> lock(_mutex);
    _cv.wait(lock, [this](){ return _count > 0; });
    --_count;
}

bool Semaphore::tryWait()
{
    std::unique_lock<std::mutex> lock(_mutex);
    if(_count > 0)
    {
        --_count;
        return true;
    }
    return false;
}

bool Semaphore::waitFor(const std::chrono::milliseconds &time)
{
    std::unique_lock<std::mutex> lock(_mutex);
    auto result = _cv.wait_for(lock, time, [this](){ return _count > 0;});
    if(result)
        --_count;
    return result;
}

bool Semaphore::waitUntil(const std::chrono::time_point<std::chrono::high_resolution_clock> &time_point)
{
    std::unique_lock<std::mutex> lock(_mutex);
    auto result = _cv.wait_until(lock, time_point, [this](){ return _count > 0;});
    if(result)
        --_count;
    return result;
}
} // namesapce Toy

