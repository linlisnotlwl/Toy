#pragma once
#include <unordered_map>
#include <mutex>
#include "FdContext.h"
#include "Singleton.h"
namespace Toy
{
class FdManager
{
public:
    FdManager();
    FdContext::Ptr getFdCtx(int fd);
    FdContext::Ptr createFdCtx(int fd, FdContext::FdType fd_type, SocketAttribute sock_attribute);
    FdContext::Ptr dup(int oldfd, int newfd);
    void closeFdCtx(int fd);
    
private:
    FdContext::Ptr delFdCtx(int fd);
    std::unordered_map<int, FdContext::Ptr> m_all_fds;
    std::mutex m_mutex;
};

typedef Singleton<FdManager> FdMgr;
} // namespace Toy