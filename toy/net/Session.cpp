#include "Session.h"


namespace Toy
{
namespace net
{


void TCPSession::start()
{
    // 建立一个等待接收数据的协程
    // 建立一个等待发送数据的协程
}
void TCPSession::startSend()
{
    // 在协程里面运行
    // 发送文件，切出协程，发送完毕后再回到现场
}
void TCPSession::startRecv()
{
    // 在协程里面运行
    // 接收文件，切出协程，再回到现场
}

} // namespace net
} // namespace Toy