#include "poll_server.h"
#include "tool.h"
void init_pollserver(const char *ip, __uint16_t port, struct PollServer **server)
{
    *server = (struct PollServer *)malloc(sizeof(struct PollServer));
    if (server != NULL)
    {
        (*server)->listenSock = 0;
        (*server)->listenSock = mySocket();
        (*server)->nfds = NFDS;
        (*server)->port = port;
        myBind((*server)->listenSock, port, ip);
        myListen((*server)->listenSock);
        (*server)->pfd = (struct pollfd *)malloc(sizeof(struct pollfd) * (*server)->nfds);
        for (int i = 0; i < (*server)->nfds; i++)
        {
            (*server)->pfd[i].fd = FD_NONE;
            (*server)->pfd[i].events = (*server)->pfd[i].revents = 0;
        }
        (*server)->pfd[0].fd = (*server)->listenSock; // 第0个位置留给监听描述符
        (*server)->pfd[0].events = POLLIN;            // 关心listenSock的读
    }
    else
    {
        logMessage(FATAL, FILENAME, LINE, "PollServer无法被分配内存");
        exit(EXIT_FAILURE);
    }
}
void showFds_pollserver(struct PollServer *server)
{
    printf("fds:: ");
    for (int i = 0; i < server->nfds; i++)
    {
        if (server->pfd[i].fd == FD_NONE)
            continue;
        printf("%d ", server->pfd[i].fd);
    }
    printf("\n"); // 输出换行符以刷新缓冲区并换行
}
void start_pollserver(struct PollServer *server)
{
    while (1)
    {
        showFds_pollserver(server);
        logMessage(DEBUG, FILENAME, LINE, "即将调用poll检测文件描述符集是否有文件描述符就绪...");
        int res = poll(server->pfd, server->nfds, 2000); // poll的第三个参数如果为-1,poll会一直阻塞直到有事件发生
        if (res < 0)
        {
            // error
            logMessage(ERROR, FILENAME, LINE, strerror(errno));
            free_pollserver(server);
            exit(EXIT_FAILURE);
        }
        else if (res == 0)
        {
            // time out
            logMessage(DEBUG, FILENAME, LINE, "调用poll函数时超时", res);
        }
        else
        {
            // fd is ready
            logMessage(DEBUG, FILENAME, LINE, "文件描述符[%d]就绪", res);
            eventHandler_pollserver(server);
            printf("是否继续检测(y/n):");
            fflush(stdout);
            char option;
        scan:
            scanf("%c", &option);
            clearInputBuffer();
            if (option == 'y')
            {
                continue;
            }
            else if (option == 'n')
            {
                break;
            }
            else
            {
                printf("输入错误，请重新输入：");
                fflush(stdout);
                goto scan;
            }
        }
    }
}
void eventHandler_pollserver(struct PollServer *server)
{
    for (int i = 0; i < server->nfds; i++)
    {
        if (server->pfd[i].fd == FD_NONE)
            continue;
        /*
        按位与操作符 &：这会检查 revents 中是否包含 POLLIN 标志，而不管 revents 中
        是否还有其他标志。POLLIN 的值与 revents 按位与操作结果不为零，表示这个事件
        发生了
        */
        if (server->pfd[i].revents & POLLIN)
        {
            if (i == 0)
            {
                // 监听套接字的情况
                accepter_pollserver(server);
            }
            else
            {
                reader_pollserver(i, server);
            }
        }
    }
}
void accepter_pollserver(struct PollServer *server)
{
    char clientIp[16] = {'\0'};
    uint16_t clientPort;
    int sock = myAccept(server->listenSock, clientIp, &clientPort);
    // 为accept得到的套接子找一个位置放入
    int pos = 1;
    for (; pos < server->nfds; pos++)
    {
        if (server->pfd[pos].fd == FD_NONE)
            break;
    }
    if (pos == server->nfds)
    {
        // 找到最后一个位置都没找到，可以采取的措施是对server->pfd这个数组进行一个扩容
        logMessage(DEBUG, FILENAME, LINE, "文件描述符集合已满，无法再管理新的连接");
        close(sock);
    }
    else
    {
        // 找到了位置的情况
        logMessage(DEBUG, FILENAME, LINE, "connected to a new client[%d]", sock);
        server->pfd[pos].fd = sock;
        server->pfd[pos].events = POLLIN; // 关注新链接的描述符的可读状态
    }
}
void reader_pollserver(int pos, struct PollServer *server)
{
    char buf[128] = {'\0'};
    int res = read(server->pfd[pos].fd, buf, sizeof(buf));
    if (res > 0)
    {
        // SUCCESS
        printf("client message:%s\n", buf);
        // memset(buf,'\0',sizeof(buf));//清空缓冲区
    }
    else if (res == 0)
    {
        // EOF 关闭文件描述符
        printf("client closed\n");
        close(server->pfd[pos].fd);
        server->pfd[pos].fd = FD_NONE;
        server->pfd[pos].events = server->pfd[pos].revents = 0;
    }
    else
    {
        // read error
        printf("read error,errno[%d], error message:%s", errno, strerror(errno));
        close(server->pfd[pos].fd);
        server->pfd[pos].fd = FD_NONE;
        server->pfd[pos].events = server->pfd[pos].revents = 0;
    }
}
void free_pollserver(struct PollServer *server)
{
    if (server != NULL)
    {
        if (server->listenSock >= 0)
            close(server->listenSock);
        if (server->pfd != NULL)
        {
            free(server->pfd);
            server->pfd = NULL;
        }
        free(server);
    }
}
