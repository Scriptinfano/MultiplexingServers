#pragma once
#include "log.h"
#include <stdlib.h>
#include <sys/socket.h>
#include <err.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
extern const int gBackLog; // 连接就绪队列
int mySocket();
void myBind(int listenSock, u_int16_t port, const char *ip);
void myListen(int listenSock);
/*
int listenSock: 监听套接字的文件描述符
char *clientIP：输出型参数，获取连接之后的对端IP
uint16_t *clientPort：输出型参数，获取连接之后的对端端口
*/
int myAccept(int listenSock, char *clientIP, uint16_t *const clientPort);