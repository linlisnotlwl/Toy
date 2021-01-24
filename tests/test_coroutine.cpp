#include "ToyCo.h"
#include "Timer.h"

static Toy::TinyTimer global_timer;

static const int YEILD_COUNT = 1000000;

static void fun_one(int id)
{
    printf("ID(%d) start in fun_one.\n", id);
    TOY_CO_YEILD;
    //printf("ID(%d) yeild once in fun_one.\n", id);
    int k = 0;
    for(int i = 0; i < YEILD_COUNT; ++i)
    {
        TOY_CO_YEILD;
        k++;
    }
    printf("ID(%d) finish in fun_one.\n", id);
}

static void test_co_yeild()
{
    TOY_CO_CREATE( [](){fun_one(1);} );
    TOY_CO_CREATE( [](){fun_one(2);} );
    TOY_CO_CREATE( [](){fun_one(3);} );
    TOY_CO_CREATE( [](){fun_one(4);} );
}

static int golbal_short_fun_count = 0;
static const int SHORT_FUN_CREATE_NUM = 100000;
static void short_fun(int id)
{
    //printf("ID(%d) start in short_fun.\n", id);
    ++golbal_short_fun_count;
    TOY_CO_YEILD;
    printf("ID(%d) finish in short_fun.\n", id);
}
static void test_co_create()
{
    for(int i = 0; i < SHORT_FUN_CREATE_NUM; ++i)
    {
        TOY_CO_CREATE( [i](){short_fun(i);} );
    }
    printf("test_co_create finished!");// count = %d.\n", golbal_short_fun_count
}

static const int SUSPEND_NUM = 100;
static void suspend_fun(int id, uint64_t time)
{
    printf("ID(%d) start in suspend_fun.\n", id);
    TOY_CO_SUSPEND(time);
    TOY_CO_YEILD;
    //printf("ID(%d) suspend once in suspend_fun.\n", id);
    int k = 0;

    for(int i = 0; i < SUSPEND_NUM; ++i)
    {
        TOY_CO_SUSPEND(time);
        TOY_CO_YEILD;
        k++;
    }

    printf("ID(%d) finish in suspend_fun.%d\n", id, k);

    //目前的问题：cur_co老是等于nullptr，如何存储suspend后的上下文？？suspend 后 yeild
    // TODO: 是否应该将yeild合并进行suspend中？
}

static void test_co_suspend()
{

    TOY_CO_CREATE( [](){suspend_fun(1, 1);} );
    TOY_CO_CREATE( [](){suspend_fun(2, 10);} );
    TOY_CO_CREATE( [](){suspend_fun(3, 100);} );
    //TOY_CO_CREATE( [](){suspend_fun(4, 1000);} );
}

static Toy::Cohandler::SuspendInfo global_si;
static void wait_for_wakeup()
{
    printf("!Waiting for a handsome man to wake me up\n");
    TOY_CO_SUSPEND_NOWAKUP(global_si);
    TOY_CO_YEILD;
    printf("!I have been waken up.Thank you! mua! (*╯3╰)\n");

}

static void wakeup_waiting()
{
    TOY_CO_SUSPEND(1000);
    TOY_CO_YEILD;
    printf("co address = %d\n", global_si.sus_co);
    TOY_CO_WAKEUP(global_si); 
}

static void test_wakeup()
{
    TOY_CO_CREATE(wait_for_wakeup);
    TOY_CO_CREATE(wakeup_waiting);
}
int main()
{
    //test_co_yeild();
    //test_co_create();
    //test_co_suspend();
    test_wakeup();

    printf("START!!!\n");
    TOY_CO_START;
    return 0;
}