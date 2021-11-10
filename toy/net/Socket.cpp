#include "Socket.h"
#include "../Util.h"
#include "../Log.h"
#include <sys/socket.h>



namespace Toy
{
namespace net
{

Socket::Ptr Socket::createTCPSocket(Address::Ptr addr)
{
    Socket::Ptr ret = std::make_shared<Socket>(addr->getFamily(), Type::TCP);
    if(!ret->isSockFdVaild())
        return nullptr;
    ret->bind(addr);
    return ret;
}

Socket::Ptr Socket::createUDPSocket(Address::Ptr addr)
{
    Socket::Ptr ret = std::make_shared<Socket>(addr->getFamily(), Type::UDP);
    if(!ret->isSockFdVaild())
        return nullptr;
    ret->bind(addr);
    return ret;
}

Socket::Socket(Domain domain, Type type, int protocol)
{
    m_sockfd = socket(domain, type, protocol);
    if(m_sockfd == -1)
    {
        TOY_LOG_ERROR << "create sockfd error. errno = "
            << errno << "(" << strerror(errno) << ")";
    }
    m_domain = domain;
    m_type = type;
}

Socket::Socket(int fd, Domain domain, Type type, int protocol) 
    : m_sockfd(fd), m_domain(domain), m_type(type)
{

}

bool Socket::bind(Address::Ptr addr)
{
    if(addr == nullptr)
        return false;
    // if(addr->getFamily() != AF_INET && addr->getFamily() != AF_INET6)
    //     return false;
    int ret = ::bind(m_sockfd, addr->getAddr(), addr->getAddrLength());
    if(ret == -1)
    {
        TOY_LOG_ERROR << "Socket bind error. errno = "
            << errno << "(" << strerror(errno) << ")";
        return false;
    }

    return true;
}

bool Socket::listen(int backlog)
{
    int ret = ::listen(m_sockfd, backlog);
    if(ret == -1)
    {
        TOY_LOG_ERROR << "Socket listen error. errno = "
            << errno << "(" << strerror(errno) << ")";
        return false;
    }
    return true;
}

Socket::Ptr Socket::accept()
{
    int new_sock = ::accept(m_sockfd, nullptr, nullptr);
    if(new_sock == -1)
    {
        TOY_LOG_ERROR << "Socket accept error. errno = "
            << errno << "(" << strerror(errno) << ")";
        return nullptr;     
    }
    return std::make_shared<Socket>(new_sock, m_domain, m_type);
}

} // namespace net
} // namespace Toy