#include "Hook.h"

#include <dlfcn.h>


// dlsym:
// 功能是根据动态链接库操作句柄与符号，返回符号对应的地址，不但可以获取函数地址，也可以获取变量地址。
// void*dlsym(void*handle,const char*symbol)根据 动态链接库 操作句柄(handle)与符号(symbol)，返回符号对应的地址。
// 即可以获取函数地址，也可以获取变量地址

/*暂时
#define HOOK_FUN(X) \ 
    X(sleep)


namespace Toy
{

static int staticInitHook()
{
    //TODO: 判断是否要HOOK
// 获取系统函数的符号地址到对应的变量
#define SET_FUN_TO_DLSYM(name) name ## _f = (name ## _fun)dlsym(RTLD_NEXT, #name);
    HOOK_FUN(SET_FUN_TO_DLSYM);
#undef SET_FUN_TO_DLSYM
}

void initHook()
{
    // 静态变量初始化，在main调用前调用该函数 //TODO: 搞清楚这里的初始化问题！！
    static int init_statues = staticInitHook();
    (void)init_statues;
}

}

extern "C"
{
// 初始设置函数指针为NULL(全局初始化)
#define SET_FUN_TO_NULL(name) name ## _fun name ## _f = nullptr;
    HOOK_FUN(SET_FUN_TO_NULL);
#undef SET_FUN_TO_NULL


unsigned int sleep(unsigned int seconds)
{
    // TODO: 判断是否开启了HOOK
    // if(is_hook)
    //     return sleep_sys(seconds);

}
} // extern "C"


暂时*/