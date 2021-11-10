#pragma once
#include "Noncopyable.h"
#include "Address.h"
#include <memory>
namespace Toy
{
namespace net
{



class Socket : Noncopyable
{
public:
    typedef std::shared_ptr<Socket> Ptr;
    enum Domain
    {
        IPv4 = AF_INET,
        IPv6 = AF_INET6,
        UNIX = AF_UNIX        
    };
    enum Type
    {
        TCP = SOCK_STREAM,
        UDP = SOCK_DGRAM
    };
    
    static Ptr createTCPSocket(Address::Ptr addr);
    static Ptr createUDPSocket(Address::Ptr addr);
    virtual ~Socket();
    int getFd() const { return m_sockfd; }
    void shutdownWrite();
    void close();
    bool bind(Address::Ptr addr);
    bool listen(int backlog);
    Ptr accept();

    void setTCPNoDelay(bool on);
    
protected:
    explicit Socket(Domain, Type, int protocol = 0);
private:
    explicit Socket(int fd, Domain, Type, int protocol = 0);
    bool isSockFdVaild() const { return m_sockfd != -1; }
    int m_sockfd;
    Address::Ptr m_addr;
    Domain m_domain;
    Type m_type;
};



} // namespace net
} // namespace Toy