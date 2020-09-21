#include <cstdio>
#include <chrono>
#include "Log.h"
#include "ThreadPool.h"

Toy::ThreadPool threadpool(4);

constexpr int LOG_NUM = 1 * 10000;
void testLogThread(Toy::LogLevel::Level level)
{
	for (int count = 0; count < LOG_NUM; ++count)
	{
		//TOY_LOG_ERROR << "testLogThread: level = " << level << ",count = " << count;
		TOY_LOG_DEBUG << "testLogThread: level = " << level << ",count = " << count;
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

static void test_log_level()
{
	#ifndef NDEBUG
		TOY_LOG_DEBUG << "test_log_level in DEBUG.";
	#else	
		TOY_LOG_DEBUG << "test_log_level in NDEBUG.";
	#endif
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
	//test_log_level();
    auto end = std::chrono::high_resolution_clock::now();
	
    std::cout << "Time: " 
        << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() 
        << " us" << std::endl;
	
	//std::this_thread::sleep_for(std::chrono::milliseconds(5000)); // 等待线程输出所有日志
	//threadpool.shutdownnow();

	// avg : 1.5s, 400000 (str+int) msgs, virtual machine(1 Cpu, 2 core, i7-4720, 1G) 
    return 0;
}