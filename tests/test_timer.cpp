#include <thread>
#include <iostream>
#include "Timer.h"


static Toy::Timer global_timer;
static int count = 0;
void call_back_fun(Toy::Tick::Ptr tick)
{
    std::cout << count++ << ":Running CallBackFun of Tick " << tick.get()
        << ", expired_time = " << tick->getExpiredTick()
        << ", at : " << global_timer.getElapsedMillSecond() << std::endl;
}

void call_back_two(void * data)
{
    std::cout << "Running CallBackFun of Tick one, at : " 
        << global_timer.getElapsedMillSecond() << std::endl;
}

static void test_timewheel()
{
    //using namespace std::chrono_literals; // c++14
    Toy::TimerWheel tw(5, 1);
    tw.start();
    std::cout << "TimerWheel start time = " << global_timer.getElapsedMillSecond() << std::endl;
    Toy::Tick::Ptr tick1_p = std::make_shared<Toy::Tick>();
    Toy::Tick::Ptr tick2_p = std::make_shared<Toy::Tick>();
    
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "Add tick time = " << global_timer.getElapsedMillSecond() << std::endl;
    tw.add(tick1_p, 3, std::bind(call_back_fun, tick1_p), 10, 5);
    tw.add(tick2_p, 10, std::bind(call_back_fun, tick2_p), 15, 4);

    for(int i = 0; i < 30; ++i)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        tw.update();
    }
    //tw.del(tick1_p);
    for(int i = 0; i < 5000; ++i)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        tw.update();
    }


}

int main()
{
    global_timer.update();
    test_timewheel();
    return 0;
}