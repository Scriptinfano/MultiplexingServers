#include "select_server.h"
#include "poll_server.h"
#include "epoll_server.h"
#include "tool.h"
#include "log.h"
#include <stddef.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
void test_selectserver()
{
    struct SelectServer *p = initSelectServer(8080, "127.0.0.1");
    struct timeval the_timeval;
    the_timeval.tv_sec = 10;
    the_timeval.tv_usec = 0;
    startSelectServer(p, &the_timeval);
}
void test()
{
    const char *ip = "127.0.0.1";
    short clientPort = 443;
    logMessage(NORMAL, LOGPARAM, "get link from client[%s:%d]", ip, clientPort);
}
void test_pollserver()
{
    struct PollServer *server = initPollServer("127.0.0.1", 12345);
    if (server != NULL)
    {
        startPollServer(server);
        freePollServer(server);
    }
    else
    {
        logMessage(FATAL, LOGPARAM, "PollServer创建失败");
        exit(1);
    }
}
void test_epollserver()
{
    struct EpollServer *server = initEpollServer("127.0.0.1", 12345);
    startEpollServer(-1, server);
    freeEpollServer(server);
}
int main()
{
    test_pollserver();
}