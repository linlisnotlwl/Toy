#pragma once
#include <string>
#include <memory>
#include <netinet/in.h>

namespace Toy
{
namespace net
{

class Address
{
public:
    typedef std::shared_ptr<Address> Ptr;
    
    virtual ~Address() = 0;
    virtual int getFamily() const = 0;
    virtual std::string toString() const = 0;
    virtual sockaddr * getAddr() const = 0;
    virtual socklen_t getAddrLength() const = 0;
    virtual bool operator==(const Address & rhs) const;
    virtual bool operator!=(const Address & rhs) const;
};

class IPAddress : public Address
{
public:
    

    typedef std::shared_ptr<IPAddress> Ptr;
    virtual in_port_t getPort() const = 0;
    virtual void setPort(in_port_t) = 0;

protected:
    explicit IPAddress(const std::string & host_or_address, in_port_t port);
private:
    //std::string m_addr_name;
    //in_port_t port;
    //sockaddr m_addr; 
};

class IPv4Address : public IPAddress
{
public:
    typedef std::shared_ptr<IPv4Address> Ptr;

    static IPv4Address::Ptr create(const std::string & host_or_address, in_port_t port);
    //explicit IPv4Address(const std::string & host_or_address, in_port_t port);
    explicit IPv4Address(const sockaddr_in & addr);
    virtual int getFamily() const override { return AF_INET; }
    virtual in_port_t getPort() const override { return m_addr.sin_port; }
    virtual void setPort(in_port_t port) override { m_addr.sin_port = port; }
    virtual sockaddr * getAddr() const override { return (sockaddr *)&m_addr; }
    virtual socklen_t getAddrLength() const override { return sizeof(m_addr); }
private:
    sockaddr_in m_addr;
};

class IPv6Address : public IPAddress
{
public:
    typedef std::shared_ptr<IPv6Address> Ptr;

    static IPv6Address::Ptr create(const std::string & host_or_address, in_port_t port);
    //explicit IPv6Address(const std::string & host_or_address, in_port_t port);
    explicit IPv6Address(const sockaddr_in6 & addr);
    virtual int getFamily() const override { return AF_INET6; }
    virtual in_port_t getPort() const override { return m_addr.sin6_port; }
    virtual void setPort(in_port_t port) override { m_addr.sin6_port = port; }
    virtual sockaddr * getAddr() const override { return (sockaddr *)&m_addr; }
    virtual socklen_t getAddrLength() const override { return sizeof(m_addr); }
private:
    sockaddr_in6 m_addr;
};

// TODO: UnixAddress,UnknownAddress

} // namespace net

} // namespace Toy