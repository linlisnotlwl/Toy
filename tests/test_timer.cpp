#include <thread>
#include <iostream>
#include <cstdint>
#include "Timer.h"
#include "Log.h"


static Toy::TinyTimer global_timer;
static int count = 0;
static double global_sum_time = 0;
static Toy::TimerWheel global_tw;

static std::weak_ptr<Toy::Tick> watch_ptr1;
static std::weak_ptr<Toy::Tick> watch_ptr2;
static std::weak_ptr<Toy::Tick> watch_ptr3;
static std::weak_ptr<Toy::Tick> watch_ptr4;

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

void call_back_two()
{
    std::cout << "Running CallBackFun of Tick one, at : " 
        << global_timer.getElapsedMillSecond() << std::endl;
}

static void test_timewheel()
{
    //using namespace std::chrono_literals; // c++14
    //Toy::TimerWheel tw(5, 1); // 5 wheel, 1 ms interval as a tick
    Toy::TimerWheel tw;
    printf("TimerWheel in test_timewheel is %llu.\n", &tw);
    tw.start();
    {   // 放在这里用于保证后面tw.close时该作用域的智能指针已经释放
        std::cout << "TimerWheel start time = " << global_timer.getElapsedMillSecond() << std::endl;
        Toy::Tick::Ptr tick1_p = std::make_shared<Toy::Tick>();
        Toy::Tick::Ptr tick2_p = std::make_shared<Toy::Tick>();
        Toy::Tick::Ptr tick3_p = std::make_shared<Toy::Tick>();
        Toy::Tick::Ptr tick4_p = std::make_shared<Toy::Tick>();

        watch_ptr1 = tick1_p;
        watch_ptr2 = tick2_p;
        watch_ptr3 = tick3_p;
        watch_ptr4 = tick4_p;
        std::this_thread::sleep_for(std::chrono::seconds(1));
        printf("Add tick time = %f\n", global_timer.getElapsedMillSecond());
        tw.add(tick1_p, 3, std::bind(call_back_fun, tick1_p), INT32_MAX, 50);
        tw.add(tick2_p, 10, std::bind(call_back_fun, tick2_p), INT32_MAX, 4);
        tw.add(tick3_p, 1000, std::bind(call_back_fun, tick3_p), INT32_MAX, 1001);
        //tw.add(tick4_p, 10, std::bind(call_back_fun, tick4_p), INT32_MAX, 2);
        tw.add(tick4_p, 10, [](){ printf("Running tick4_p.\n"); }, INT32_MAX, 2);
        tw.add(20, [](){ printf("Running Task 4.\n"); });
    }



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
    
    printf("TW start closing.\n");
    tw.close();
    printf("TW closed.\n");
}
void threadTestFun(int id)
{
    Toy::Tick::Ptr tick1_p = std::make_shared<Toy::Tick>();
    Toy::Tick::Ptr tick2_p = std::make_shared<Toy::Tick>();
    Toy::Tick::Ptr tick3_p = std::make_shared<Toy::Tick>();
    Toy::Tick::Ptr tick4_p = std::make_shared<Toy::Tick>();

    std::this_thread::sleep_for(std::chrono::seconds(1));
    printf("Add tick time = %f\n", global_timer.getElapsedMillSecond());
    // global_tw.add(tick1_p, 3, std::bind(call_back_fun, tick1_p), INT32_MAX, 50);
    // global_tw.add(tick2_p, 10, std::bind(call_back_fun, tick2_p), INT32_MAX, 4);
    // global_tw.add(tick3_p, 1000, std::bind(call_back_fun, tick3_p), INT32_MAX, 1001);
    // global_tw.add(tick4_p, 10, std::bind(call_back_fun, tick4_p), INT32_MAX, 2);
    // NOTE: bind指针指针会导致指针的释放不符合预期！！！！程序结束时出现Memory Leak
    // TODO: 删除add（PTR）的接口，不由用户掌握tick指针。考虑给用户提供一个专属tickID
    global_tw.add(tick1_p, 3, [](){ printf("Running tick1_p.\n"); }, INT32_MAX, 50);
    global_tw.add(tick2_p, 10, [](){ printf("Running tick2_p.\n"); }, INT32_MAX, 4);
    global_tw.add(tick3_p, 1000, [](){ printf("Running tick3_p.\n"); }, INT32_MAX, 1001);
    global_tw.add(tick4_p, 10, [](){ printf("Running tick4_p.\n"); }, INT32_MAX, 2);
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

static void readd_fun(Toy::TimerWheel::TimeDuration time, int num)
{
    if(num == 0)
        return;
    --num;
    // NOTE !!!! 
    // 在任务中添加任务会造成死锁，因为add会加锁，而update时也会加锁，
    // 所以在执行任务时，是从update中的tick进入的，所以死锁了
    // 已解决，可以重添加: add中删除了锁，在每个wheel的addInSlot中添加了锁
    // FIXME: 重update，在执行任务中update定时器会造成死锁
    bool state = global_tw.add(10, [time, num](){ readd_fun(time, num); });
    if(!state)
        printf("!!!!!Add failed.\n");
    static int count = 0;
    printf("Finished readd_fun %d.\n", ++count);
}

static void test_readd()
{
    global_tw.start();
    global_tw.autoUpdate();
    readd_fun(10, 100);
    std::this_thread::sleep_for(std::chrono::milliseconds(11000));
    global_tw.close();

}

int main()
{
    TOY_LOG_DEBUG << "test timer start";
    global_timer.update();
    test_timewheel();
    test_multi_thread();
    //printf("Average latency: %f(ms)\n", global_sum_time / static_cast<double>(count));
    test_readd();
    
    // TOY_LOG_DEBUG << "weak 1 info : count = " << watch_ptr1.use_count() 
    //     << ", Expired = " << watch_ptr1.expired();
    // TOY_LOG_DEBUG << "weak 2 info : count = " << watch_ptr2.use_count() 
    //     << ", Expired = " << watch_ptr2.expired();
    // TOY_LOG_DEBUG << "weak 3 info : count = " << watch_ptr3.use_count() 
    //     << ", Expired = " << watch_ptr3.expired();
    // TOY_LOG_DEBUG << "weak 4 info : count = " << watch_ptr4.use_count() 
    //     << ", Expired = " << watch_ptr4.expired();

    TOY_LOG_DEBUG << "Waitting to exit.";
    std::this_thread::sleep_for(std::chrono::seconds(2));
    TOY_LOG_DEBUG << "EXIT!";
    return 0;
}