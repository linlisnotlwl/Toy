#pragma once
#include <functional>


#include "../Noncopyable.h"
#include "Socket.h"


// session 代表一段对话，可以是有连接的，也可以是无连接的，存储有双方的信息
// 通过session进行数据通信，可以注册读写数据完成的callback
namespace Toy
{
namespace net
{
class Session
{
public:
    typedef std::shared_ptr<Session> Ptr;
    typedef std::function<void (void *)> CallBackFun;
    virtual ~Session() {}
    virtual void start() = 0;
    virtual void send() = 0;
    virtual void shutdown() = 0;
    virtual bool isEstablished() const = 0;
    virtual int setSocketNoDelay() = 0;
    virtual const Address::Ptr & getLocalAddr() = 0;
    virtual const Address::Ptr & getRemoteAddr() = 0;
    virtual bool registerSendDone(const CallBackFun & cb);

private:

};



class TCPSession : Noncopyable, public Session
{
public:
    typedef std::shared_ptr<TCPSession> Ptr;
    TCPSession();
    virtual ~TCPSession();
    virtual void start() override;
    virtual void send() override;
    void recv();
    virtual void shutdown() override;
    virtual bool isEstablished() const override;
    virtual int setSocketNoDelay() override;
    virtual const Address::Ptr & getLocalAddr() override;
    virtual const Address::Ptr & getRemoteAddr() override;
private:
    void startSend();
    void startRecv();
    Socket::Ptr m_sock;

};


class UDPSession : Noncopyable, public Session
{
public:
    typedef std::shared_ptr<UDPSession> Ptr;
private:

};

} // namespace net
} // namespace Toy