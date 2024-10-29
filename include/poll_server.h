#pragma once
#include <poll.h>
#include <stdint.h>
#include "socket.h"
#define FD_NONE -1 // 每个文件描述符的初始化值
#define NFDS 5     // pollfd数组的元素个数

struct PollServer
{
    uint16_t port;      // 监听端口号
    int listenSock;     // 监听套接字文件描述符
    struct pollfd *pfd; // pollfd的数组
    int nfds;
};

struct PollServer *initPollServer(const char *ip, uint16_t port);
void showPollServerFds(struct PollServer *server);
void startPollServer(struct PollServer *server);
void handlePollServerEvent(struct PollServer *server);
void acceptPollServer(struct PollServer *server);
void readPollServer(int pos, struct PollServer *server);
void freePollServer(struct PollServer *server);
