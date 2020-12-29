#include <thread>
#include <iostream>
#include "Timer.h"


static Toy::TinyTimer global_timer;
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
    Toy::TimerWheel tw(5, 1); // 5 wheel, 1 ms interval as a tick
    tw.start();
    std::cout << "TimerWheel start time = " << global_timer.getElapsedMillSecond() << std::endl;
    Toy::Tick::Ptr tick1_p = std::make_shared<Toy::Tick>();
    Toy::Tick::Ptr tick2_p = std::make_shared<Toy::Tick>();
    Toy::Tick::Ptr tick3_p = std::make_shared<Toy::Tick>();

    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "Add tick time = " << global_timer.getElapsedMillSecond() << std::endl;
    tw.add(tick1_p, 3, std::bind(call_back_fun, tick1_p), 10, 50);
    tw.add(tick2_p, 10, std::bind(call_back_fun, tick2_p), 15, 4);
    tw.add(tick3_p, 1000, std::bind(call_back_fun, tick3_p), 3, 1001);
    tw.add(20, [](){ printf("Running Task 4.\n"); });


    for(int i = 0; i < 30; ++i)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        tw.update();
    }
    if(!tw.del(tick2_p))
        printf("((((((((((((((Delete tick error.%x\n", tick2_p.get());
    else
        printf("&&&&&&&&&&&&&&Delete tick success.%x\n", tick2_p.get());
    
    tw.autoUpdate();

    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    //tw.update();
    
    tw.close();
}

int main()
{
    global_timer.update();
    test_timewheel();
    return 0;
}