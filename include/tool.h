#pragma once

void clearInputBuffer();
/*
@brief 将文件描述符设为非阻塞
@param fd 文件描述符
*/
void SetNonBlock(int fd);