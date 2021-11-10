#pragma once
#include "Abstract.h" // foo Server



// 所有连接都有一个session代表
// 通过该session，可以进行读写，关闭等
// 一个TCPserver可以有多个session
// 每个session可以注册对应的读写handle函数，或者读写完成回调函数
namespace Toy
{
namespace net
{

class TCPServer : public Server
{
public:
    TCPServer();
    virtual ~TCPServer();
    virtual void start() override;
    virtual void shutdown() override;
    virtual Address::Ptr getLocalAddr() override;

private:
    void startAccept();
    bool m_is_running;

};
} // namespace net
} // namespace Toy