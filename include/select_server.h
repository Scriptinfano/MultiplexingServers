#pragma once
#include "socket.h"
#include <sys/time.h>
#include <sys/select.h>
#include <assert.h>
#define NUM 1024 // fd_set中能存1024个文件描述符
#define FD_NONE -1
struct SelectServer
{
    uint16_t port;
    int listenSock;
    int rdArray[NUM];//由于select函数的大多数参数为输入输出型参数，每次调用都必须重设参数，此时就需要一个第三方数组来保存时刻在变化的文件描述符，并用这个数组在每次select调用的时候更新其中的文件描述符最大个数参数以及文件描述符监控位图参数
};
/*
@brief 初始化select服务器
@param port 要监听的端口
@ip 要监听的ip地址
@return 返回SelectServer的地址
*/
struct SelectServer *initSelectServer(uint16_t port, const char *ip);
/*

@brief 启动SelectServer，按照指定配置，开始监听指定ip和端口
@param server SelectServer的地址
@param timeval_ptr 设置select的函数的第三个参数，影响它的阻塞性，NULL为一直阻塞，非零值为阻塞一段时间，零值为不阻塞
*/
void start_selectserver(struct SelectServer *server, struct timeval *timeval_ptr);
/*
@brief 处理文件描述符就绪事件的时候，如果遇到服务器监听描述符，则调用accept方法传入新的连接
@param server SelectServer结构体的地址
*/
static void accepter(struct SelectServer *server);
static void showFds(struct SelectServer *server);
static void eventHandler(fd_set *rfds, struct SelectServer *server);
static void reader(int fd, struct SelectServer *server);