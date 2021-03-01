#pragma once
#include <memory>
#include <sys/socket.h>
#include "ReactorElement.h"
namespace Toy
{

// 存储socket信息，用以后续判断socket的类型
struct SocketAttribute
{
    SocketAttribute() {}
    SocketAttribute(int _domain, int _type, int _protocol) 
        : domain(_domain), type(_type), protocol(_protocol) {}
    int domain = -1;
    int type = -1;
    int protocol = -1;
};

// 线程不安全
class FdContext : public ReactorElement, public std::enable_shared_from_this<FdContext>
{
public:
    enum FdType
    {
        UNKNOWN = 0,
        SOCKET,
        PIPE,
        FILE
    };
    typedef std::shared_ptr<FdContext> Ptr;
    explicit FdContext(int fd, FdType fd_type, SocketAttribute sock_attribute);
    bool isSocket() const { return m_fd_type == FdType::SOCKET; }
    bool isClose() const { return m_is_close; }
    void setNonBlock(bool is_nonblocking);
    bool isNonBlocking() const { return m_is_nonblocking; }
    bool isTCPSocket() const 
    {
        return isSocket() && m_sock_attribute.type == SOCK_STREAM;
    }
    //void reset(int fd, FdType fd_type, SocketAttribute sock_attribute);
    
    FdContext::Ptr clone(int newfd);
    void close();
    void setSocketTimeoutMicroSecond(int optname, int microseconds);
    int getSocketTimeoutMicroSecond(int optname);
    int getSocketTimeoutMillSecond(int optname);
    SocketAttribute getSocketAttribute() { return m_sock_attribute; }
    void setSocketAttribute(const SocketAttribute & sa) { m_sock_attribute = sa; }
private:
    //void setNonBlockMember(bool is_nonblocking) { m_is_nonblocking = is_nonblocking; }
    //int m_fd;
    FdType m_fd_type;
    bool m_is_close = false;
    bool m_is_nonblocking = false;
    int m_recv_timeout_us = -1;
    int m_send_timeout_us = -1;
    SocketAttribute m_sock_attribute;
};

} // namespace Toy