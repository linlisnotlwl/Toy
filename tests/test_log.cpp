#include <cstdio>
#include <chrono>
#include "Log.h"
#include "ThreadPool.h"

Toy::ThreadPool threadpool(4);

constexpr int LOG_NUM = 10 * 10000;
void testLogThread(Toy::LogLevel::Level level)
{
	for (int count = 0; count < LOG_NUM; ++count)
	{
		TOY_LOG_ERROR << "testLogThread: level = " << level << ",count = " << count;
	}
	//printf("%lu done!\n", std::this_thread::get_id());
}
void testLog()
{
	Toy::LogLevel::Level level[4] = { Toy::LogLevel::DEBUG,
	Toy::LogLevel::INFO, Toy::LogLevel::Level::ERROR, Toy::LogLevel::FATAL };
	std::thread t[4];
	for (int i = 0; i < 4; ++i)
	{
		t[i] = std::thread(testLogThread, level[i]);
		
	}
	for(int i = 0; i < 4; ++i)
	{
		t[i].join();
	}
	TOY_LOG_ERROR << "HELLO_end";
}
void testTask()
{
	static std::atomic<int> count(0);
	count++;
	TOY_LOG_DEBUG << " void testLogUsingThreadPool(); count = " << count.load() ;
}
void testLogUsingThreadPool()
{
	threadpool.start();
	for(int i = 0; i < 10; ++i)
	{
		threadpool.run(std::function<void()>(testTask));
	}	
}

static void test_log_cmd()
{

}
static void test_log_file()
{
    
}

int main()
{


	auto start = std::chrono::high_resolution_clock::now();
	testLog();
	//testLogUsingThreadPool();
    auto end = std::chrono::high_resolution_clock::now();
	
    std::cout << "Time: " 
        << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() 
        << " us" << std::endl;
	
	std::this_thread::sleep_for(std::chrono::milliseconds(5000)); // 等待线程输出所有日志
	//threadpool.shutdownnow();
    return 0;
}