#include "ThreadPool.h"
#include "Log.h"
namespace Toy
{
ThreadPool::ThreadPool(size_t thread_num, const std::string &name) 
    : m_thread_size(thread_num), m_name(name), m_is_running(false)
{

}

ThreadPool::~ThreadPool()
{
    shutdownnow();
}

void ThreadPool::start()
{
    if(m_is_running.load(std::memory_order_acquire) == false)
    {
        m_is_running.store(true, std::memory_order_release);
        for(size_t i = 0; i < m_thread_size; ++i)
        {
            m_threads.emplace_back( new Thread(std::mem_fn(&ThreadPool::runningThread), this) );
        }
        TOY_LOG_DEBUG << "ThreadPool::start() tid = " << std::this_thread::get_id;
    }
}

void ThreadPool::run(Task task)
{
    if(m_is_running.load(std::memory_order_acquire) == false)
        task();
    else
    {
        std::unique_lock<std::mutex> lock(m_queue_mutex);
        m_task_queue.push_back(std::move(task));
        if(m_task_queue.size() == 1)
            m_queue_not_empty.notify_one();
    }
    
}

void ThreadPool::shutdown()
{
    m_is_running.store(false, std::memory_order_seq_cst);
    m_queue_not_empty.notify_all();
    for(size_t i = 0; i < m_threads.size(); ++i)
    {
        m_threads[i]->detach();
    }
    m_threads.clear();
    m_task_queue.clear();
    
}

void ThreadPool::shutdownnow()
{
    m_is_running.store(false, std::memory_order_seq_cst);
    m_queue_not_empty.notify_all();
    for(size_t i = 0; i < m_threads.size(); ++i)
    {
        if(m_threads[i]->joinable())
            m_threads[i]->join();
    }
    
    m_threads.clear();
    m_task_queue.clear();
    
}

void ThreadPool::runningThread()
{
    while(m_is_running.load(std::memory_order_acquire) == true)
    {
        
        Task task;
        {
            std::unique_lock<std::mutex> lock(m_queue_mutex);
            if(m_task_queue.empty())
                m_queue_not_empty.wait(lock, [this](){ return !m_task_queue.empty() || 
                    m_is_running.load(std::memory_order_acquire) == false; });
            if(!m_task_queue.empty())
            {
                task = m_task_queue.front();
                m_task_queue.pop_front();
            }
            else
                task = nullptr;
        }
        if(task)
            task();
    }
}

} // namespace Toy