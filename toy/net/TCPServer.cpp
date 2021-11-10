#include "TCPServer.h"

namespace Toy
{
namespace net
{
void TCPServer::start()
{

}
void TCPServer::shutdown()
{
    
}
void TCPServer::startAccept()
{
    // 当前函数应该分配到一个协程中处理？？
    // 循环
    // 调用accept获取新sock
    // 根据sock建立新session
    // 将新session添加进集合
    // 
}

} // namespace net
} // namespace Toy