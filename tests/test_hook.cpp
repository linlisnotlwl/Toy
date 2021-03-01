#include "Hook.h"
#include "Timer.h"
#include "ToyCo.h"
#include "Log.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <netdb.h> // for gethostbyname

Toy::TinyTimer global_timer;

static void test_sleep()
{
    global_timer.update();
    printf("sleep_sys address = %llu\n", sleep_sys);
    //sleep_sys(3);
    sleep(10);
    printf("time = %f(s)\n", global_timer.getElapsedSecond());
    printf("sleep_sys address = %llu\n", sleep_sys);
    TOY_CO_END;
}

static void test_sock()
{
    TOY_LOG_INFO << "--------------------test sock begin-----------------------";
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0)
    {
        TOY_LOG_ERROR << "socket create error. sock = " << sock << ". errno = " << strerror(errno);
        return;
    }
    else
        TOY_LOG_INFO << "socket fd = "<< sock;
    //hostent * baidu_host = gethostbyname("www.baidu.com");


    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    //addr.sin_addr.s_addr = *(in_addr_t *)baidu_host->h_addr;
    addr.sin_addr.s_addr = inet_addr("202.108.22.5");
    addr.sin_port = htons(80);

    //TOY_LOG_INFO << "Baidu IPv4 address :" << inet_ntoa(addr.sin_addr);

    int res = connect(sock, (const sockaddr *)&addr, sizeof(addr));
    if(res < 0)
    {
        TOY_LOG_ERROR << "connect error. res = " << res << ". errno = " << strerror(errno);
        TOY_CO_END;
        return;
    }
    else
        TOY_LOG_INFO << "connect success";
    

    const char data[] = "GET / HTTP/1.0\r\n\r\n";
    res = send(sock, data, sizeof(data), 0); // 默认阻塞
    if(res <= 0)
    {
        TOY_LOG_ERROR << "send error. res = " << res << ". errno = " << strerror(errno);
        TOY_CO_END;
        return;
    }
    else
        TOY_LOG_INFO << "Send success!!!!";

    std::string buff;
    buff.resize(4096);

    res = recv(sock, &buff[0], buff.size(), 0);
    TOY_LOG_INFO << "recv result = " << res << ". errno = " << strerror(errno);

    if(res <= 0)
    {
        TOY_CO_END;
        return;
    }
       
    
    buff.resize(res);
    TOY_LOG_INFO << "获取到的网页信息：" << buff;
    TOY_LOG_INFO << "--------------------test sock end-----------------------";
    TOY_CO_END;
}
static void test_log()
{
    TOY_LOG_INFO << "test_log";
    TOY_CO_END;
}

int main()
{
    // TOY_LOG_DEBUG << "???";
    // TOY_LOG_INFO << "why???";
    //Toy::enableHook();
    //Toy::initHook();
    //TOY_CO_CREATE(test_sleep);
    TOY_CO_CREATE(test_sock);
    //TOY_CO_CREATE(test_log);
    TOY_CO_START;
    return 0;
}