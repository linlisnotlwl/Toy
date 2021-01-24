#include "Hook.h"
#include "ToyCo.h"

#include <dlfcn.h>


static thread_local bool IS_HOOK = false;

// dlsym:
// 功能是根据动态链接库操作句柄与符号，返回符号对应的地址，不但可以获取函数地址，也可以获取变量地址。
// void*dlsym(void*handle,const char*symbol)根据 动态链接库 操作句柄(handle)与符号(symbol)，返回符号对应的地址。
// 即可以获取函数地址，也可以获取变量地址

//对每个函数应用X宏. 注意：、后面不要有空格！！！
#define HOOK_FUN(X) \
    X(sleep) \
    X(usleep) \
    X(nanosleep) \
    X(socket) \
    X(connect) \
    X(accpet) \
    X(read) \
    X(readv) \
    X(recv) \
    X(recvfrom) \
    X(recvmsg) \
    X(write) \
    X(writev) \
    X(send) \
    X(sendto) \
    X(sendmsg) \
    X(close) \ 
    X(fcntl) \
    X(ioctl) \
    X(getsockopt) \
    X(setsockopt) \


namespace Toy
{

static int staticInitHook()
{
    //TODO: 判断是否要HOOK

// 获取系统函数的符号地址到对应的变量
#define SET_FUN_TO_DLSYM(name) name ## _sys = (name ## _fun)dlsym(RTLD_NEXT, #name);
    HOOK_FUN(SET_FUN_TO_DLSYM);
#undef SET_FUN_TO_DLSYM
    return 0;
}

void initHook()
{
    // 静态变量初始化，在main调用前调用该函数 //TODO: 搞清楚这里的初始化问题！！
    //static int init_statues = 
    staticInitHook();
    //(void)init_statues;
    
}


void enableHook()
{
    IS_HOOK = true;
}
void disableHook()
{
    IS_HOOK = false;
}
bool isHook()
{
    return IS_HOOK;
}

} // namespace Toy

extern "C"
{
// 初始设置函数指针为NULL(全局初始化) 
#define SET_FUN_TO_NULL(name) name ## _fun name ## _sys = nullptr;
    HOOK_FUN(SET_FUN_TO_NULL);
#undef SET_FUN_TO_NULL



// RETURE: 0 或者 当被信号中断时，剩下的时间
// MT-Unsafe sig:SIGCHLD/linux 
// On Linux, sleep() is implemented via nanosleep(2).
unsigned int sleep(unsigned int seconds)
{
    if(sleep_sys == nullptr)
        Toy::initHook();
    if(!IS_HOOK)
    {
        return sleep_sys(seconds);
    }
    
    auto cur_co = Toy::Cohandler::getCurCoroutine();
    if(cur_co == nullptr)
        return sleep_sys(seconds);
    printf("Call my sleep\n");
    TOY_CO_SUSPEND(seconds * 1000);
    TOY_CO_YEILD;
    return 0;
}

// RETURE: 0成功，-1失败
// ERRORS:
//      EINTR  Interrupted by a signal; see signal(7).
//      EINVAL usec is greater than or equal to 1000000.  (On systems where that is considered an error.)
// 线程安全
int usleep(useconds_t usec)
{

}

// If the call is interrupted by a signal handler, nanosleep() returns -1,
// sets errno to EINTR, and writes the remaining time into the structure pointed to by
// rem unless rem is NULL.  The value of *rem can then be used to call nanosleep() 
// again and complete the specified pause (but see NOTES).
//            struct timespec {
//                time_t tv_sec;        /* seconds */
//                long   tv_nsec;       /* nanoseconds */
//            };
// The value of the nanoseconds field must be in the range 0 to 999999999.
// 相比sleep和usleep：
//     1、精度更高
//     2、POSIX.1明确规定不与信号交互？？
//     3、当被信号中断时，能更容易的重新进入睡眠
// ERRORS：
//     EFAULT Problem with copying information from user space.
//     EINTR  The pause has been interrupted by a signal that was delivered to the thread (see signal(7)).  The remaining sleep time has been written into *rem so
//            that the thread can easily call nanosleep() again and continue with the pause.
//     EINVAL The value in the tv_nsec field was not in the range 0 to 999999999 or tv_sec was negative.
// NOTE: 看不太懂，到时候再回来看看
int nanosleep(const struct timespec *req, struct timespec *rem)
{

}


int socket(int domain, int type, int protocol)
{

}

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{

}

int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{

}

ssize_t read(int fd, void *buf, size_t count)
{

}

ssize_t readv(int fd, const struct iovec *iov, int iovcnt)
{

}
ssize_t recv(int sockfd, void *buf, size_t len, int flags)
{

}

ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, 
    struct sockaddr *src_addr, socklen_t *addrlen)
{

}

ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags)
{

}

ssize_t write(int fd, const void *buf, size_t count)
{

}

ssize_t writev(int fd, const struct iovec *iov, int iovcnt)
{

}

ssize_t send(int sockfd, const void *buf, size_t len, int flags)
{

}

size_t sendto(int sockfd, const void *buf, size_t len, int flags,
    const struct sockaddr *dest_addr, socklen_t addrlen)
{

}

ssize_t sendmsg(int sockfd, const struct msghdr *msg, int flags)
{

}

int fcntl(int fd, int cmd, ... /* arg */ )
{

}

int ioctl(int fd, unsigned long request, ...)
{

}

int getsockopt(int sockfd, int level, int optname,
    void *optval, socklen_t *optlen)
{

}

int setsockopt(int sockfd, int level, int optname,
    const void *optval, socklen_t optlen)
{

}

int close(int fd)
{
    
}

} // extern "C"


