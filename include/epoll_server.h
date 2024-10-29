#pragma once
#include <sys/epoll.h>
#define MAX_EVENTS 10
struct EpollServer
{
    int listenSock;              // 监听的文件描述符
    uint16_t port;               // 监听的端口号
    int epfd;                    // epoll实例的文件描述符
    struct epoll_event *revents; // 就绪事件列表
    int maxRevents;              // 最大就绪事件数
};
int initEpollModel();
void initEpollServer(const char *ip, uint16_t port, struct EpollServer **server);
void startEpollServer(struct EpollServer *server);
void controlEpoll(int epfd, int op, int fd, uint32_t events);
int waitEpoll(int epfd, struct epoll_event *events, int maxevents, int timeout);
void freeEpollServer(struct EpollServer *server);
void acceptEpollServer(struct EpollServer *server);
void readEpollServer(int sock, struct EpollServer *server);
void handleEpollServerEvent(int n, struct EpollServer *server);