#include <cstdio>
#include "Log.h"

void testLogThread(Toy::LogLevel::Level level)
{

	for (int count = 0; count < 100000; ++count)
	{
		TOY_LOG_ERROR << level << " HELLO:" << count;
	}
	printf("%lu done!\n", std::this_thread::get_id());
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
	TOY_LOG_INFO << "testing";
    auto end = std::chrono::high_resolution_clock::now();

    std::cout << "Time: " 
        << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() 
        << " us" << std::endl;
	
    return 0;
}