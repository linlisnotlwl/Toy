#pragma once
#include <thread>
#include <condition_variable>
#include <functional> // for function
#include <deque>
#include <memory> // for unique_ptr
#include <vector>
#include <atomic>
#include <mutex>
#include <functional> // for mem_fn
#include <memory> // for smart pointer

#include "Noncopyable.h"
//#include "Semaphore.h"
//__cplusplus >= 201103L
namespace Toy
{

class ThreadPool : Noncopyable
{
public:
    typedef std::thread Thread;
    typedef std::function<void ()> Task;
    typedef std::deque<Task> Queue;
    ThreadPool(size_t thread_num, const std::string & name = "TheadPool");
    ~ThreadPool();

    // you have to call start to use ThreadPool after init or shutdown
    void start();
    void shutdown();
    void shutdownnow();

    void run(Task);
    size_t getTaskNum() const { return m_task_queue.size(); }
    std::string getName() const { return m_name; }
private:
    void runningThread();
    size_t m_thread_size;
    //size_t m_max_queue_size;
    Queue m_task_queue;
    std::vector<std::unique_ptr<Thread>> m_threads;
    std::string m_name;
    //Semaphore m_queue_not_empty;
    std::condition_variable m_queue_not_empty;
    //Semaphore m_is_queue_full;

    std::mutex m_queue_mutex;
    
    std::atomic<bool> m_is_running;
};

} // namespace Toy
