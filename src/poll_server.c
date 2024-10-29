#include "poll_server.h"
#include "socket.h"
#include "tool.h"
#include "log.h"
#include <stddef.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
/*
@brief 初始化PollServer结构体，返回创建的结构体的指针
@param ip 要监听的ip地址
@param port 要监听的端口号
@return 返回创建的结构体指针，如果创建失败了，返回NULL
*/
PollServer *initPollServer(const char *ip, uint16_t port)
{
    PollServer *server = (PollServer *)malloc(sizeof(PollServer));
    if (server != NULL)
    {
        server->listenSock = mySocket();
        server->nfds = NFDS;//最多可以监视几个文件描述符
        server->port = port;
        myBind(server->listenSock, port, ip);
        myListen(server->listenSock);
        server->pfd = (struct pollfd *)malloc(sizeof(struct pollfd) * server->nfds);
        for (int i = 0; i < server->nfds; i++)
        {
            server->pfd[i].fd = FD_NONE;
            server->pfd[i].events = server->pfd[i].revents = 0;
        }
        server->pfd[0].fd = server->listenSock; // 第0个位置留给监听描述符
        server->pfd[0].events = POLLIN;         // 关心listenSock的读
    }
    else
    {
        logMessage(FATAL, LOGPARAM, "PollServer无法被分配内存");
        return NULL;
    }
    return server;
}
void showPollServerFds(PollServer *server)
{
    printf("fds:: ");
    for (int i = 0; i < server->nfds; i++)
    {
        if (server->pfd[i].fd == FD_NONE)
            continue;
        printf("%d ", server->pfd[i].fd);
    }
    printf("\n"); // 输出换行符以刷新缓冲区并换行
}
void startPollServer(PollServer *server)
{
    while (1)
    {
        showPollServerFds(server);
        int res = poll(server->pfd, server->nfds, 2000); // poll的第三个参数如果为-1,poll会一直阻塞直到有事件发生
        if (res < 0)
        {
            // error
            logMessage(FATAL, LOGPARAM, strerror(errno));
            freePollServer(server);
            exit(EXIT_FAILURE);
        }
        else if (res == 0)
        {
            // time out
            logMessage(DEBUG, LOGPARAM, "poll在指定时间段内没有检测到就绪的文件描述符，发生超时");
        }
        else
        {
            // fd is ready
            logMessage(DEBUG, LOGPARAM, "文件描述符[%d]就绪", res);
            handlePollServerEvent(server);
            printf("是否继续检测(y/n):");
            fflush(stdout);
            char option;
        scan:
            scanf("%c", &option);
            clearInputBuffer();
            if (option == 'y')
            {
                continue;
            }
            else if (option == 'n')
            {
                break;
            }
            else
            {
                printf("输入错误，请重新输入：");
                fflush(stdout);
                goto scan;
            }
        }
    }
}
void handlePollServerEvent(PollServer *server)
{
    for (int i = 0; i < server->nfds; i++)
    {
        if (server->pfd[i].fd == FD_NONE)
            continue;
        /*
        按位与操作符 &：这会检查 revents 中是否包含 POLLIN 标志，而不管 revents 中
        是否还有其他标志。POLLIN 的值与 revents 按位与操作结果不为零，表示这个事件
        发生了
        */
        if (server->pfd[i].revents & POLLIN)
        {
            if (i == 0)
            {
                // 监听套接字的情况
                acceptPollServer(server);
            }
            else
            {
                readPollServer(i, server);
            }
        }
    }
}
void acceptPollServer(PollServer *server)
{
    char clientIp[16] = {'\0'};
    uint16_t clientPort;
    int sock = myAccept(server->listenSock, clientIp, &clientPort);
    // 为accept得到的套接子找一个位置放入
    int pos = 1;
    for (; pos < server->nfds; pos++)
    {
        if (server->pfd[pos].fd == FD_NONE)
            break;
    }
    if (pos == server->nfds)
    {
        // 找到最后一个位置都没找到，可以采取的措施是对server->pfd这个数组进行一个扩容
        logMessage(DEBUG, LOGPARAM, "文件描述符集合已满，无法再管理新的连接");
        close(sock);
    }
    else
    {
        // 找到了位置的情况
        logMessage(DEBUG, LOGPARAM, "connected to a new client[%d]", sock);
        server->pfd[pos].fd = sock;
        server->pfd[pos].events = POLLIN; // 关注新链接的描述符的可读状态
    }
}
void readPollServer(int pos, PollServer *server)
{
    char buf[128] = {'\0'};
    int res = read(server->pfd[pos].fd, buf, sizeof(buf));
    if (res > 0)
    {
        // SUCCESS
        printf("client message:%s\n", buf);
        // memset(buf,'\0',sizeof(buf));//清空缓冲区
    }
    else if (res == 0)
    {
        // EOF 关闭文件描述符
        printf("client closed\n");
        close(server->pfd[pos].fd);
        server->pfd[pos].fd = FD_NONE;
        server->pfd[pos].events = server->pfd[pos].revents = 0;
    }
    else
    {
        // read error
        printf("read error,errno[%d], error message:%s", errno, strerror(errno));
        close(server->pfd[pos].fd);
        server->pfd[pos].fd = FD_NONE;
        server->pfd[pos].events = server->pfd[pos].revents = 0;
    }
}
void freePollServer(PollServer *server)
{
    if (server != NULL)
    {
        if (server->listenSock >= 0)
            close(server->listenSock);
        if (server->pfd != NULL)
        {
            free(server->pfd);
            server->pfd = NULL;
        }
        free(server);
    }
}
