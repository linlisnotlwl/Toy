#include "FdContext.h"
#include <unistd.h>


namespace Toy
{

FdContext::FdContext(int fd, FdType fd_type, SocketAttribute sock_attribute) 
    : ReactorElement(fd), m_fd_type(fd_type), m_sock_attribute(sock_attribute)
{

}

FdContext::Ptr FdContext::clone(int newfd)
{
    if(newfd <= 0 || m_is_close)
        return nullptr;
    FdContext::Ptr ret_ctx = std::make_shared<FdContext>(newfd, m_fd_type, m_sock_attribute);
    ret_ctx->m_is_nonblocking = m_is_nonblocking;
    ret_ctx->m_is_close = m_is_close;
    ret_ctx->m_recv_timeout_us = m_recv_timeout_us;
    ret_ctx->m_send_timeout_us = m_send_timeout_us;
    // TODO: 注意不要漏了
    return ret_ctx;
}

void FdContext::close()
{
    m_is_close = true;
    trigger(nullptr, IOEvent::NVAL);
    //::close(m_fd); 由close(hook版)函数里调用
}

void FdContext::setSocketTimeoutMicroSecond(int optname, int microseconds)
{
    if(optname == SO_RCVTIMEO)
        m_recv_timeout_us = microseconds;
    else if(optname == SO_SNDTIMEO)
        m_send_timeout_us = microseconds;
}

int FdContext::getSocketTimeoutMicroSecond(int optname)
{
    if(optname == SO_RCVTIMEO)
        return m_recv_timeout_us;
    else if(optname == SO_SNDTIMEO)
        return m_send_timeout_us;
    else
        return 0;
}

int FdContext::getSocketTimeoutMillSecond(int optname)
{
    if(optname == SO_RCVTIMEO)
        return m_recv_timeout_us * 1000;
    else if(optname == SO_SNDTIMEO)
        return m_send_timeout_us * 1000;
    else
        return 0;
}

} // namespace Toy