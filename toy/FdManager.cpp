#include "FdManager.h"
#include "Log.h"


namespace Toy
{

FdManager::FdManager()
{

}

FdContext::Ptr FdManager::getFdCtx(int fd)
{
    if(fd < 0)
        return nullptr;
    std::unique_lock<std::mutex> lock(m_mutex);
    auto it = m_all_fds.find(fd);
    if(it == m_all_fds.end())
    {
        return nullptr;
    }
    else
        return it->second;
}

FdContext::Ptr FdManager::createFdCtx(int fd, FdContext::FdType fd_type, SocketAttribute sock_attribute)
{
    if(fd < 0)
        return nullptr;
    std::unique_lock<std::mutex> lock(m_mutex);
    auto it = m_all_fds.find(fd);
    if(it == m_all_fds.end())
    {
        auto ptr = std::make_shared<FdContext>(fd, fd_type, sock_attribute);
        m_all_fds[fd] = ptr;
        TOY_LOG_DEBUG << "Create FdCtx in FdMgr. fd = " << ptr->getFd() << ", real fd = " << fd;
        return ptr;
    }
    else // TODO: reset fdctx
        return it->second;
}

FdContext::Ptr FdManager::dup(int oldfd, int newfd)
{
    if(oldfd <= 0 || newfd <= 0)
        return nullptr;
    std::unique_lock<std::mutex> lock(m_mutex);
    auto it = m_all_fds.find(oldfd);
    if(it != m_all_fds.end())
    {
        auto ret = it->second->clone(newfd);
        // 这里直接替代就行，因为所有的关闭与复制已经由系统函数完成
        m_all_fds[newfd] = ret;
        return ret;
    }
    else 
        return nullptr;
}

void FdManager::closeFdCtx(int fd)
{
    auto fd_ctx = delFdCtx(fd);
    if(!fd_ctx->isClose())
        fd_ctx->close();
}

FdContext::Ptr FdManager::delFdCtx(int fd)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    auto it = m_all_fds.find(fd);

    if(it != m_all_fds.end())
    {
        m_all_fds.erase(it);
        return it->second;
    }
    else
        return nullptr;
}

} // namespace Toy