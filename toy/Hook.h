#pragma once

// 线程级别的HOOK


// 在进入main函数前，执行一段函数：
// 声明一个全局类变量，在调用main之前会先初始化该变量，所以会调用该类的构造函数



namespace Toy
{
void initHook();
} // namespace Toy


extern "C"
{

typedef unsigned int (*sleep_fun)(unsigned int seconds);
extern sleep_fun sleep_sys;

} // extern "C"