#pragma once
#include <stdint.h>
#include "socket.h"
#include <sys/epoll.h>
#define MAX_EVENTS 10
int init_epollmodel();
struct EpollServer
{
    int listenSock;              // 监听的文件描述符
    uint16_t port;               // 监听的端口号
    int epfd;                    // epoll实例的文件描述符
    struct epoll_event *revents; // 就绪事件列表
    int maxRevents;              // 最大就绪事件数
};
void init_epollserver(const char *ip, uint16_t port, struct EpollServer **server);
void start_epollserver(struct EpollServer *server);
void ctl_epoll(int epfd, int op, int fd, uint32_t events);
int wait_epoll(int epfd, struct epoll_event *events, int maxevents, int timeout);
void free_epollserver(struct EpollServer *server);
void accepter_epollserver(struct EpollServer *server);
void reader_epollserver(int sock, struct EpollServer *server);
void handler_epollserver(int n, struct EpollServer *server);