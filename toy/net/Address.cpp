#include "Address.h"
#include "Log.h"
#include <netdb.h> // for getaddrinfo
#include <string.h>
namespace Toy
{
namespace net
{
IPv4Address::Ptr IPv4Address::create(const std::string & host_or_address, in_port_t port)
{
    addrinfo hints, **result;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    // TODO: getaddrinfo有可能阻塞，应该添加hook版本, 或者阻止域名获取，只使用IP地址 hints.ai_flags = AI_NUMERICHOST;
    int error = getaddrinfo(host_or_address.c_str(), 0, &hints, result);
    if(error != 0)
    {
        TOY_LOG_DEBUG << "Get IPv4 addr info error" 
            << "(Addr:" << host_or_address 
            << ",port:" << port << ")."
            << "errno = " << errno << "," 
            << "(" << strerror(errno) << ")."
            << "return error = " << error
            << "(" << gai_strerror(error) << ").";
        return nullptr;
    }

    IPv4Address::Ptr ret = std::make_shared<IPv4Address>(*reinterpret_cast<sockaddr_in *>((*result)->ai_addr));
    ret->m_addr.sin_port = port;
    freeaddrinfo(*result);
    return ret;
}

IPv4Address::IPv4Address(const sockaddr_in & addr) : m_addr(addr)
{

}

IPv6Address::Ptr IPv6Address::create(const std::string & host_or_address, in_port_t port)
{
    addrinfo hints, **result;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET6;
    // TODO: getaddrinfo有可能阻塞，应该添加hook版本, 或者阻止域名获取，只使用IP地址 hints.ai_flags = AI_NUMERICHOST;
    int error = getaddrinfo(host_or_address.c_str(), 0, &hints, result);
    if(error != 0)
    {
        TOY_LOG_DEBUG << "Get IPv6 addr info error" 
            << "(Addr:" << host_or_address 
            << ",port:" << port << ")."
            << "errno = " << errno << "," 
            << "(" << strerror(errno) << ")."
            << "return error = " << error
            << "(" << gai_strerror(error) << ").";
        return nullptr;
    }

    IPv6Address::Ptr ret = std::make_shared<IPv6Address>(*reinterpret_cast<sockaddr_in *>((*result)->ai_addr));
    ret->m_addr.sin6_port = port;
    freeaddrinfo(*result);
    return ret;
}

IPv6Address::IPv6Address(const sockaddr_in6 & addr) : m_addr(addr)
{

}
    
} // namespace net

} // namespace Toy