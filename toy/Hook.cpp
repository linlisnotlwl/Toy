#include "Hook.h"
#include "ToyCo.h"
#include "FdManager.h"
#include "Log.h"
#include "Reactor.h"

#include <dlfcn.h>
#include <stdarg.h>


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
    X(accept) \
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

/**
 * @brief IO复用函数，用于监控IO事件
 * 
 * @param fd_ctx 
 * @param event 要监控是事件
 * @param revent 触发的事件？？TODO
 * @param time_out 超时时间ms(小于等于零表示不设置超时时间)
 * @return int -1：错误；1：超时；0：事件触发
 */
int IOMultiplexing(FdContext::Ptr fd_ctx, IOEvent event, 
    IOEvent & revent, int time_out)
{
    if(fd_ctx == nullptr)
    {
        errno = EBADF;
        return -1;
    }
    
    

    // 设置event
    
    Reactor::Entry entry;
    if(time_out > 0)
        entry.si = Cohandler::suspend(static_cast<TimerWheel::TimeDuration>(time_out));
    else
        entry.si = Cohandler::suspend();
    // 添加event到Reactor，进行实践监控
    if(!Reactor::getReactor(fd_ctx->getFd()).add(fd_ctx->getFd(), event, entry))
    {
        TOY_LOG_DEBUG << "Reactor add event error";
        //Cohandler::wakeup(entry.si);
        return -1;
    }
    
    TOY_LOG_DEBUG << "Co_yeild. wakeup state = " << entry.si->wakeup_state;
    // yeild出，等待唤醒
    TOY_CO_YEILD;


    // 达到唤醒条件：有设定的事件发生，或者到时了 // TODO好像不用删除定时任务或删除事件了，因为唤醒时会自动判断是否已经唤醒了
    if(entry.si->wakeup_state == 1) // 到时
    {
        //if(!Reactor::getReactor(fd_ctx->getFd()).delEvent(fd_ctx->getFd(), event, )
        // 超时则要记录超时状态
        revent = IOEvent::TIMEOUT; // TODO:是否要根据Reactor来确定
        return 1;
    }
    else if(entry.si->wakeup_state == 2)// 事件触发时，要删除定时任务（不用了）
    {
        revent = event; // TODO: 是否要根据Reactor来确定
        return 0;
    }
    else
    {
        TOY_LOG_ERROR << "Wakeup State is wrong.";
        return -1;
    }
}

/**
 * @brief Hook部分有较为简单IO事件的函数
 * 
 * @tparam OriginFun 被Hook的原始系统IO函数
 * @tparam Args OriginFun所需要的参数
 * @param sockfd socket文件描述符
 * @param fun 被Hook的原始系统IO函数
 * @param hook_fun_name 函数的名字
 * @param event 要监控的事件
 * @param timeout_type 超时类型 SO_RCVTIMEO SO_SNDTIMEO
 * @param args fun需要的剩下的参数
 * @return int 
 */
template <typename OriginFun, typename ... Args>
static int IOHook(int sockfd, OriginFun fun, const char * hook_fun_name,
    IOEvent event, int timeout_type, Args && ... args)
{

    auto cur_co = Cohandler::getCurCoroutine();
    if(cur_co == nullptr)
        return fun(sockfd, std::forward<Args>(args)...); // 完美转发

    FdContext::Ptr fd_ctx = FdMgr::getInstance().getFdCtx(sockfd);
    if(fd_ctx == nullptr || fd_ctx->isNonBlocking())
        return fun(sockfd, std::forward<Args>(args)...);
    
    // 处理超时时间
    int time_out = fd_ctx->getSocketTimeoutMillSecond(timeout_type);

    // 添加事件, 等待对应的IO事件触发
    IOEvent revent;
re_wait:
    //TODO:是否要记录下已经等待时间，用作下次更新
    int state = IOMultiplexing(fd_ctx, event, revent, time_out);
    if(state == -1)
    {
        if(errno == EINTR)
            goto re_wait;
        else
            return -1;
    }
    else if(state == 1) // 超时
    {
        errno = ETIMEDOUT; // EAGAIN？？
        return -1;
    }
    // 事件触发，调用对应的非阻塞IO函数
re_call: //TODO:这里是否还需要重试，这里的中断是否应该反映给用户
    auto ret = fun(sockfd, std::forward<Args>(args)...);
    if(-1 == ret)
    {
        if(errno == EINTR)
            goto re_call;
        else
            return -1;
    }
    return ret;
}

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
    if(usleep_sys == nullptr)
        Toy::initHook();
    if(!IS_HOOK)
    {
        return usleep_sys(usec);
    }
    
    auto cur_co = Toy::Cohandler::getCurCoroutine();
    if(cur_co == nullptr || usec >= 1000000)
        return usleep_sys(usec); 
    TOY_CO_SUSPEND(usec);
    TOY_CO_YEILD;
    return 0;
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
    if(nanosleep_sys == nullptr)
        Toy::initHook();
    if(!IS_HOOK)
        return nanosleep_sys(req, rem);
    //TOY_LOG_DEBUG << "call hook nanosleep.";
    return nanosleep_sys(req, rem);
    //TODO: 先改造TimerWheel，实现更小的粒度，目前只支持到ms
}


int socket(int domain, int type, int protocol)
{
    if(socket_sys == nullptr)
        Toy::initHook();
    if(!IS_HOOK)
        return socket_sys(domain, type, protocol);
    //auto cur_co = Toy::Cohandler::getCurCoroutine();
    int sock_fd = socket_sys(domain, type, protocol);
    if(sock_fd >= 0)
        Toy::FdMgr::getInstance().createFdCtx(sock_fd, 
            Toy::FdContext::FdType::SOCKET, Toy::SocketAttribute(domain, type, protocol));
    return sock_fd; 
}

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    if(connect_sys == nullptr)
        Toy::initHook();
    if(!IS_HOOK)
        return connect_sys(sockfd, addr, addrlen);

    auto cur_co = Toy::Cohandler::getCurCoroutine();
    if(cur_co == nullptr)
        return connect_sys(sockfd, addr, addrlen);

    Toy::FdContext::Ptr fd_ctx = Toy::FdMgr::getInstance().getFdCtx(sockfd);
    if(fd_ctx == nullptr)
        return connect_sys(sockfd, addr, addrlen);
    if(fd_ctx->isClose())
    {
        errno = EBADF;
        return -1;
    }
    if(fd_ctx->isNonBlocking() || !fd_ctx->isTCPSocket()) // 这里暂时只处理TCP的connect，其他类型的具体得看看手册
        return connect_sys(sockfd, addr, addrlen);
    
    // 调用非阻塞的connect
    fd_ctx->setNonBlock(true);
    int res = connect_sys(sockfd, addr, addrlen);
    TOY_LOG_DEBUG << "NonBlock System connect return = " << res;
    fd_ctx->setNonBlock(false);



    //    EINPROGRESS
    //           The  socket  is  nonblocking and the connection cannot be completed immedi?
    //           ately.  (UNIX domain sockets failed with EAGAIN instead.)  It  is  possible
    //           to select(2) or poll(2) for completion by selecting the socket for writing.
    //           After select(2) indicates writability, use getsockopt(2) to read the SO_ER?
    //           ROR  option  at  level  SOL_SOCKET to determine whether connect() completed
    //           successfully (SO_ERROR is zero) or unsuccessfully (SO_ERROR is one  of  the
    //           usual error codes listed here, explaining the reason for the failure).
    if(res == 0)
        return 0;
    else if(res != -1 || errno != EINPROGRESS)
        return res;

    TOY_LOG_DEBUG << "connecting in EINPROGRESS";
    
    // 满足下面条件则往下走   
    //if(res == -1 && errno == EINPROGRESS) //TODO&& errno != EAGAIN 不用了，前面已经判断了是socket了，但没判断是TCPSocket啊 
    static const int CONNECT_TIMEOUT = -1; //ms
    Toy::IOEvent revent;
    res = Toy::IOMultiplexing(fd_ctx, Toy::IOEvent::OUT, revent, CONNECT_TIMEOUT);

    if(res == -1 || revent == Toy::IOEvent::TIMEOUT)
    {
        errno = ETIMEDOUT;
        return -1;
    }
    int error = 0;
    socklen_t solen = sizeof(error);
    res = getsockopt_sys(sockfd, SOL_SOCKET, SO_ERROR, &error, &solen);
    if(res == -1)
        return -1;
    if(error == 0)
        return 0;
    else
    {
        errno = error;
        return -1;
    } 

}

int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
    if(accept_sys == nullptr)
        Toy::initHook();
    if(!IS_HOOK)
        return accept(sockfd, addr, addrlen);
    Toy::FdContext::Ptr fd_ctx = Toy::FdMgr::getInstance().getFdCtx(sockfd);

    if(fd_ctx == nullptr)
    {
        errno = EBADF;
        return -1;
    }
    //static const int ACCEPT_TIMEOUT = 10;   //ms
    int accept_sock = Toy::IOHook(sockfd, accept_sys, 
        "accept", Toy::IOEvent::IN, SO_RCVTIMEO, addr, addrlen);
    if(accept_sock >= 0)
        Toy::FdMgr::getInstance().createFdCtx(accept_sock,
            Toy::FdContext::FdType::SOCKET, fd_ctx->getSocketAttribute());
    return accept_sock;
}

ssize_t read(int fd, void *buf, size_t count)
{
    if(read_sys == nullptr)
        Toy::initHook();
    if(!IS_HOOK)
        return read_sys(fd, buf, count);
    //static const int READ_TIMEOUT = -1;   //ms
    return Toy::IOHook(fd, read_sys, 
        "read", Toy::IOEvent::IN, SO_RCVTIMEO, buf, count);
}

ssize_t readv(int fd, const struct iovec *iov, int iovcnt)
{
    if(readv_sys == nullptr)
        Toy::initHook();
    if(!IS_HOOK)
        return readv_sys(fd, iov, iovcnt);
    //static const int READV_TIMEOUT = -1;   //ms
    return Toy::IOHook(fd, readv_sys, 
        "readv", Toy::IOEvent::IN, SO_RCVTIMEO, iov, iovcnt);
}

ssize_t recv(int sockfd, void *buf, size_t len, int flags)
{
    if(recv_sys == nullptr)
        Toy::initHook();
    if(!IS_HOOK)
        return recv_sys(sockfd, buf, len, flags);
    //static const int RECV_TIMEOUT = -1;   //ms
    return Toy::IOHook(sockfd, recv_sys, 
        "recv", Toy::IOEvent::IN, SO_RCVTIMEO, buf, len, flags);
}

ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, 
    struct sockaddr *src_addr, socklen_t *addrlen)
{
    if(recvfrom_sys == nullptr)
        Toy::initHook();
    if(!IS_HOOK)
        return recvfrom_sys(sockfd, buf, len, flags, src_addr, addrlen);
    return Toy::IOHook(sockfd, recvfrom_sys, "recvfrom",
        Toy::IOEvent::IN, SO_RCVTIMEO, buf, len, flags, src_addr, addrlen);
}

ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags)
{
    if(recvmsg_sys == nullptr)
        Toy::initHook();
    if(!IS_HOOK)
        return recvmsg_sys(sockfd, msg, flags);
    return Toy::IOHook(sockfd, recvmsg_sys, "recvmsg",
        Toy::IOEvent::IN, SO_RCVTIMEO, msg, flags);
}

ssize_t write(int fd, const void *buf, size_t count)
{
    if(write_sys == nullptr)
        Toy::initHook();
    if(!IS_HOOK)
        return write_sys(fd, buf, count);
    return Toy::IOHook(fd, write_sys, "write",
        Toy::IOEvent::OUT, SO_SNDTIMEO, buf, count);
}

ssize_t writev(int fd, const struct iovec *iov, int iovcnt)
{
    if(writev_sys == nullptr)
        Toy::initHook();
    if(!IS_HOOK)
        return writev_sys(fd, iov, iovcnt);
    return Toy::IOHook(fd, writev_sys, "writev",
        Toy::IOEvent::OUT, SO_SNDTIMEO, iov, iovcnt);
}

ssize_t send(int sockfd, const void *buf, size_t len, int flags)
{
    if(send_sys == nullptr)
        Toy::initHook();
    if(!IS_HOOK)
        return send_sys(sockfd, buf, len, flags);
    return Toy::IOHook(sockfd, send_sys, "send",
        Toy::IOEvent::OUT, SO_SNDTIMEO, buf, len, flags);
}

ssize_t sendto(int sockfd, const void *buf, size_t len, int flags,
    const struct sockaddr *dest_addr, socklen_t addrlen)
{
    if(sendto_sys == nullptr)
        Toy::initHook();
    if(!IS_HOOK)
        return sendto_sys(sockfd, buf, len, flags, dest_addr, addrlen);
    return Toy::IOHook(sockfd, sendto_sys, "sendto",
        Toy::IOEvent::OUT, SO_SNDTIMEO, buf, len, flags, dest_addr, addrlen);
}

ssize_t sendmsg(int sockfd, const struct msghdr *msg, int flags)
{
    if(sendmsg_sys == nullptr)
        Toy::initHook();
    if(!IS_HOOK)
        return sendmsg_sys(sockfd, msg, flags);
    return Toy::IOHook(sockfd, sendmsg_sys, "sendmsg",
        Toy::IOEvent::OUT, SO_SNDTIMEO, msg, flags);
}

int fcntl(int fd, int cmd, ... /* arg */ )
{
    if(fcntl_sys == nullptr)
        Toy::initHook();
    va_list va;
    va_start(va, cmd);

    // 只处理与fd的创建和其阻塞状态相关的命令
    switch(cmd)
    {
        // void
        case F_GETFL:
        {
            va_end(va);
            int file_status_flags = fcntl_sys(fd, cmd);
            // 不用改变状态结果了，具体看F_SETFL
            return file_status_flags;
        }
        // int
        case F_SETFL:
        {
            int arg = va_arg(va, int);
            va_end(va);
            if(arg & O_NONBLOCK) // 设置了非阻塞
            {
                Toy::FdContext::Ptr fd_ctx = Toy::FdMgr::getInstance().getFdCtx(fd);
                if(fd_ctx != nullptr)
                    fd_ctx->setNonBlock(true);
            }
            // 全部设置为非阻塞，是否应该？？
            // 不应该，因为有些系统函数应该被阻塞调用，然后等待事件发生的
            // else  
            //      arg |= O_NONBLOCK;
            
            return fcntl_sys(fd, cmd, arg);
        }
        
        // int
        case F_DUPFD: 
        case F_DUPFD_CLOEXEC:
        {
            int input_newfd = va_arg(va, int);
            va_end(va);
            // 根据手册，新fd的值总是大于旧的
            int new_fd = fcntl_sys(fd, cmd, input_newfd);
            if(new_fd <= 0)
                return new_fd;
            Toy::FdMgr::getInstance().dup(fd, new_fd);
            return new_fd;
        }

        // void
        case F_GETFD://目前只有一种文件描述符状态FD_CLOEXEC，标志着执行exec时该文件描述符会不会主动关闭
        // since linux2.6.35 
        // 如果出现报错，则可在所有include之前#define _GNU_SOURCE
        // 这里通过判断进行处理
#ifdef F_GETPIPE_SZ
        case F_GETPIPE_SZ:
#endif 
        case F_GETLEASE:
        case F_GETSIG:
        case F_GETOWN:
        case F_GETOWN_EX:
        {
            va_end(va);
            return fcntl_sys(fd, cmd);
        }


        // int
        case F_SETFD:
#ifdef F_SETPIPE_SZ
        case F_SETPIPE_SZ: // 同上
#endif
        case F_NOTIFY:
        case F_SETLEASE:
        case F_SETSIG:
        case F_SETOWN:
        case F_SETOWN_EX:
        {
            int arg = va_arg(va, int);
            va_end(va);
            return fcntl_sys(fd, cmd, arg);
        }

        // struct flock *
        case F_SETLK:
        case F_SETLKW:
        case F_GETLK:
        // non-POSIX
        // case F_OFD_SETLK:
        // case F_OFD_SETLKW:
        // case F_OFD_GETLK:
        {
            struct flock * arg = va_arg(va, struct flock *);
            va_end(va);
            return fcntl_sys(fd, cmd, arg);
        }

        default:
        {
            va_end(va);
            return fcntl_sys(fd, cmd);
        }


    }

     
}

int ioctl(int fd, unsigned long request, ...)
{
    if(ioctl_sys == nullptr)
        Toy::initHook();
    va_list va;
    va_start(va, request);
    void * arg = va_arg(va, void *);
    va_end(va);

    if(FIONBIO == request)
    {
        Toy::FdContext::Ptr fd_ctx = Toy::FdMgr::getInstance().getFdCtx(fd);
        if(fd_ctx != nullptr)
        {
            bool is_nonblocking = !!*static_cast<int *>(arg);
            fd_ctx->setNonBlock(is_nonblocking);
        }            
    }
    return ioctl(fd, request, arg);
}

int getsockopt(int sockfd, int level, int optname,
    void *optval, socklen_t *optlen)
{
    if(getsockopt_sys == nullptr)
        Toy::initHook();
    return getsockopt_sys(sockfd, level, optname, optval, optlen);
}

int setsockopt(int sockfd, int level, int optname,
    const void *optval, socklen_t optlen)
{
    if(setsockopt_sys == nullptr)
        Toy::initHook();
    int ret = setsockopt_sys(sockfd, level, optname, optval, optlen);
    if(!IS_HOOK || ret != 0 || level != SOL_SOCKET)
        return ret;
    if(SO_RCVTIMEO == optname || SO_SNDTIMEO == optname)
    {
        Toy::FdContext::Ptr fd_ctx = Toy::FdMgr::getInstance().getFdCtx(sockfd);
        if(fd_ctx != nullptr)
        {
            const timeval & tv = *(const timeval *)optval;
            int microseconds = tv.tv_sec * 1000000 + tv.tv_usec;
            fd_ctx->setSocketTimeoutMicroSecond(optname, microseconds);
        }
    }
    return ret;
    
}

int close(int fd)
{
    if(close_sys == nullptr)
        Toy::initHook();
    if(!IS_HOOK)
        return close_sys(fd);
    Toy::FdMgr::getInstance().closeFdCtx(fd);
    return close_sys(fd);   
}

} // extern "C"


