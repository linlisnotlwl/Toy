#pragma once
#include "Address.h"


namespace Toy
{
namespace net
{
class Server
{
public:
    virtual ~Server();
    virtual void start() = 0;
    virtual void shutdown() = 0;
    virtual Address::Ptr getLocalAddr() = 0;
};

class Client
{
public:
    virtual ~Client();
    virtual int connect(Address::Ptr) = 0;
    
};

} // namespace net
} // namespace Toy