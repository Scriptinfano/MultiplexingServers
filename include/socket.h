#pragma once
#include <stdint.h>
static const int gBackLog = 20; // 将变量限定在本文件之内，且不可修改
int mySocket();
void myBind(int listenSock, uint16_t port, const char *ip);
void myListen(int listenSock);
/*
int listenSock: 监听套接字的文件描述符
char *clientIP：输出型参数，获取连接之后的对端IP
uint16_t *clientPort：输出型参数，获取连接之后的对端端口
*/
int myAccept(int listenSock, char *clientIP, uint16_t *const clientPort);