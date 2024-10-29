#pragma once
#include <poll.h>
#include <stdint.h>
#include "socket.h"
#define FD_NONE -1 // 每个文件描述符的初始化值
#define NFDS 5     // pollfd数组的元素个数

typedef struct PollServer
{
    uint16_t port;      // 监听端口号
    int listenSock;     // 监听套接字文件描述符
    struct pollfd *pfd; // pollfd的数组
    int nfds;
}PollServer;

PollServer *initPollServer(const char *ip, uint16_t port);
void showPollServerFds(PollServer *server);
void startPollServer(PollServer *server);
void handlePollServerEvent(PollServer *server);
void acceptPollServer(PollServer *server);
void readPollServer(int pos, PollServer *server);
void freePollServer(PollServer *server);
