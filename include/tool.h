#pragma once

void clearInputBuffer();
/*
@brief 将文件描述符设为非阻塞
@param fd 文件描述符
*/
void SetNonBlock(int fd);
/*
@brief 将整型转换为字符串
*/
char *intToString(int num);