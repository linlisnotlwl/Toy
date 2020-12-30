#include <thread>
#include <iostream>
#include <cstdint>
#include "Timer.h"


static Toy::TinyTimer global_timer;
static int count = 0;
static double global_sum_time = 0;
static Toy::TimerWheel global_tw;
void call_back_fun(Toy::Tick::Ptr tick)
{
    uint64_t tick_time = tick->getExpiredTick();
    double cur_time = global_timer.getElapsedMillSecond();
    count++;
    // std::cout << count++ << ":Running CallBackFun of Tick " << tick.get()
    //     << ", expired_time = " << tick_time
    //     << ", at : " << cur_time << std::endl;
    global_sum_time += cur_time - static_cast<double>(tick_time);
}

void call_back_two(void * data)
{
    std::cout << "Running CallBackFun of Tick one, at : " 
        << global_timer.getElapsedMillSecond() << std::endl;
}

static void test_timewheel()
{
    //using namespace std::chrono_literals; // c++14
    //Toy::TimerWheel tw(5, 1); // 5 wheel, 1 ms interval as a tick
    Toy::TimerWheel tw;
    tw.start();
    std::cout << "TimerWheel start time = " << global_timer.getElapsedMillSecond() << std::endl;
    Toy::Tick::Ptr tick1_p = std::make_shared<Toy::Tick>();
    Toy::Tick::Ptr tick2_p = std::make_shared<Toy::Tick>();
    Toy::Tick::Ptr tick3_p = std::make_shared<Toy::Tick>();
    Toy::Tick::Ptr tick4_p = std::make_shared<Toy::Tick>();

    std::this_thread::sleep_for(std::chrono::seconds(1));
    printf("Add tick time = %f\n", global_timer.getElapsedMillSecond());
    tw.add(tick1_p, 3, std::bind(call_back_fun, tick1_p), INT32_MAX, 50);
    tw.add(tick2_p, 10, std::bind(call_back_fun, tick2_p), INT32_MAX, 4);
    tw.add(tick3_p, 1000, std::bind(call_back_fun, tick3_p), INT32_MAX, 1001);
    tw.add(tick4_p, 10, std::bind(call_back_fun, tick4_p), INT32_MAX, 2);
    tw.add(20, [](){ printf("Running Task 4.\n"); });


    // for(int i = 0; i < 30; ++i)
    // {
    //     std::this_thread::sleep_for(std::chrono::milliseconds(3));
    //     tw.update();
    // }
    // if(!tw.del(tick2_p))
    //     printf("((((((((((((((Delete tick error.%x\n", tick2_p.get());
    // else
    //     printf("&&&&&&&&&&&&&&Delete tick success.%x\n", tick2_p.get());
    
    tw.autoUpdate();

    std::this_thread::sleep_for(std::chrono::seconds(10));
    //tw.update();
    
    tw.close();
}
void threadTestFun(int id)
{
    Toy::Tick::Ptr tick1_p = std::make_shared<Toy::Tick>();
    Toy::Tick::Ptr tick2_p = std::make_shared<Toy::Tick>();
    Toy::Tick::Ptr tick3_p = std::make_shared<Toy::Tick>();
    Toy::Tick::Ptr tick4_p = std::make_shared<Toy::Tick>();

    std::this_thread::sleep_for(std::chrono::seconds(1));
    printf("Add tick time = %f\n", global_timer.getElapsedMillSecond());
    global_tw.add(tick1_p, 3, std::bind(call_back_fun, tick1_p), INT32_MAX, 50);
    global_tw.add(tick2_p, 10, std::bind(call_back_fun, tick2_p), INT32_MAX, 4);
    global_tw.add(tick3_p, 1000, std::bind(call_back_fun, tick3_p), INT32_MAX, 1001);
    global_tw.add(tick4_p, 10, std::bind(call_back_fun, tick4_p), INT32_MAX, 2);
    global_tw.add(20, [id](){ printf("Running Task %d.\n", id); });

    for(int32_t i = 0; i < 5000; ++i)
    {
        std::this_thread::sleep_for(std::chrono::microseconds(1000));
        global_tw.update();
    }
}
static const int THREAD_NUM = 4;
static void test_multi_thread()
{
    global_tw.start();
    std::thread ts[THREAD_NUM];
    for(int i = 0; i < THREAD_NUM; ++i)
    {
        ts[i] = std::thread(threadTestFun, i);
    }
    for(int i = 0; i < THREAD_NUM; ++i)
    {
        if(ts[i].joinable())
            ts[i].join();
    }
    global_tw.close();
}

int main()
{
    global_timer.update();
    //test_timewheel();
    test_multi_thread();
    printf("Average latency: %f(ms)\n", global_sum_time / static_cast<double>(count));
    return 0;
}