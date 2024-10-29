#pragma once
#include <sys/epoll.h>
#include "my_hashtable.h"
#define MAX_EVENTS 10
#define MAX_BUFSIZE 1024 //定义最大的接收缓冲区大小
typedef struct EpollServer
{
    int listenSock;              // 监听的文件描述符
    uint16_t port;               // 监听的端口号
    int epfd;                    // epoll实例的文件描述符
    struct epoll_event *revents; // 就绪事件列表
    int maxRevents;              // 最大就绪事件数
    HashTable *sock_con_table;//存放sock与对应的connection对象地址之间的映射
}EpollServer;
/* 
负责管理服务器和客户端的每一个连接，内置有接收缓冲区和写入缓冲区，还有对应通信的soc。
每个sock通信的时候要处理的事件有读、写、异常，所以内部还有三个函数指针指向遇到这些事件时所对应的处理函数
*/
struct Connection;
typedef void (*func_t)(struct Connection *);
typedef struct Connection{
    int sock;
    char recv_buf[MAX_BUFSIZE];
    char send_buf[MAX_BUFSIZE];
    func_t read_callback;
    func_t send_callback;
    func_t except_callback;
    struct EpollServer *prsvr;
}Connection;

//TODO Connection的各个函数待定义
Connection *createConnection(int sock,EpollServer*prsvr);

int initEpollModel();
EpollServer *initEpollServer(const char *ip, uint16_t port);
void startEpollServer(int timeout, EpollServer *server);
void controlEpoll(int op, int fd, uint32_t events, func_t *callback_arr, EpollServer *server);
int waitEpoll(int timeout, EpollServer *server);
void freeEpollServer(struct EpollServer *server);
void acceptEpollServer(struct EpollServer *server);
void readEpollServer(int sock, struct EpollServer *server);
void handleEpollServerEvent(int n, struct EpollServer *server);