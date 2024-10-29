#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include "log.h"
#include "tool.h"
void clearInputBuffer()
{
    int c;
    // 读取并丢弃输入缓冲区中的字符，直到遇到换行符或文件结束符为止
    while ((c = getchar()) != '\n' && c != EOF)
    {
    }
}

void SetNonBlock(int fd)
{
    int old_flag;
    if ((old_flag = fcntl(fd, F_GETFD)) < 0){
        logMessage(ERROR, FILENAME, LINE, create_error_message("fcntl set failed"));
        exit(1);
    }

    fcntl(fd, F_SETFD, old_flag | O_NONBLOCK);
}

char *intToString(int num)
{
    // 动态分配内存以容纳字符串
    char *str = (char *)malloc(20 * sizeof(char)); // 假设最多 20 位
    if (str != NULL)
    {
        snprintf(str, 20, "%d", num); // 使用 snprintf 安全转换
    }
    return str; // 返回指向字符串的指针
}