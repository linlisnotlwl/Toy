#pragma once
#include "Noncopyable.h"
#include "Socket.h"
namespace Toy
{
namespace net
{


class Acceptor : Noncopyable
{
public:
    explicit Acceptor(Address::Ptr listen_addr);
    ~Acceptor();
    void listen();
    bool isListenning() const { return is_listenning }
private:
    Socket::Ptr m_sock;
    bool is_listenning;

};


} // namespace net
} // namespace Toy