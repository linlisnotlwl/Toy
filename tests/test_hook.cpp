#include "Hook.h"
#include "Timer.h"
#include "ToyCo.h"

Toy::TinyTimer global_timer;

static void test_sleep()
{
    global_timer.update();
    printf("sleep_sys address = %ul\n", sleep_sys);
    //sleep_sys(3);
    sleep(3);
    printf("time = %f(s)\n", global_timer.getElapsedSecond());
    printf("sleep_sys address = %ul\n", sleep_sys);
    TOY_CO_END;
}

int main()
{

    //Toy::enableHook();
    //Toy::initHook();
    TOY_CO_CREATE(test_sleep);
    TOY_CO_START;
    return 0;
}