#include "Acceptor.h"


namespace Toy
{

namespace net
{
Acceptor::Acceptor(Address::Ptr listen_addr)
    : m_sock(Socket::createTCPSocket(listen_addr)),
    is_listenning(false)
{

}
Acceptor::~Acceptor()
{

}

void Acceptor::listen()
{
    is_listenning = true;
    
}

} // namespace net
} // namespace Toy