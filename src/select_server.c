#include "select_server.h"

struct SelectServer *initSelectServer(uint16_t port, const char *ip)
{
    size_t server_size = sizeof(struct SelectServer);
    struct SelectServer *p = (struct SelectServer *)malloc(server_size);
    p->listenSock = mySocket();
    myBind(p->listenSock, port, ip);
    myListen(p->listenSock);

    for (int i = 0; i < NUM; i++)
    {
        p->rdArray[i] = FD_NONE;
    }
    p->rdArray[0] = p->listenSock;//listensock也要作为被监视的文件描述符，稍后会加入select函数
    return p;
}
void start_selectserver(struct SelectServer *server,struct timeval *timeval_ptr)
{
    // 声明一个文件描述符集合
    fd_set rfds;
    while (1)
    {
        // 每次循环打印一下数组中有效的fd
        showFds(server);
        // 将文件描述符集合清空
        FD_ZERO(&rfds);
        // 将文件描述符加入文件描述符集，这个过程必须写在循环中，因为select返回之后会将未就绪描述符对应的位 置为0
        FD_SET(server->listenSock, &rfds);

        // 找到最大的文件描述符，顺便将rdArray存储的文件描述符设置到fd_set中
        int maxFd = server->listenSock;
        for (int i = 0; i < NUM; i++)
        {
            if (server->rdArray[i] == FD_NONE)
                continue;
            if (maxFd < server->rdArray[i])
                maxFd = server->rdArray[i];
            FD_SET(server->rdArray[i], &rfds);
        }

        // int select(int __nfds, fd_set *__restrict__ __readfds, fd_set *__restrict__ __writefds, fd_set *__restrict__ __exceptfds, struct timeval *__restrict__ __timeout)
        //__nfds：最大文件描述符+1，select要管很多文件描述符
        //__timeout：有三种可能：永远等，直到有一个描述符就绪后才返回，将其设为空指针即可;强制等待一段时间，即使某些描述符已经准备好了，设置timeval的两个值为非零值，返回后会被设为剩下的时间）;根本不等待，不管准备好没有一律返回，将timeval的两个值设为0
        // select函数返回就绪的文件描述符数量，或0表示超时(设置了timeval的情况下)，或-1表示出错
        // rfds本质上是一个整数数组，有三个整数，每个整数的每一个二进制位的索引代表文件描述符编号
        int n = select(maxFd + 1, &rfds, NULL, NULL, timeval_ptr);

        switch (n)
        {
        case 0:
            logMessage(DEBUG, FILENAME, LINE, "time out");
            break;
        case -1:
            logMessage(ERROR, FILENAME, LINE, "select err,errno:%d, strerrno:%s", errno, strerror(errno));
            break;
        default:
            if(FD_ISSET(server->listenSock,&rfds))
                logMessage(NORMAL, FILENAME, LINE, "listensock is ready");
            eventHandler(&rfds, server);
            break;
        }
    }
}

//下面的私有函数相当于C语言中的私有函数不可被外部调用

static void reader(int fd, struct SelectServer *server)
{
    char buff[128] = {0};
    ssize_t res = read(server->rdArray[fd], buff, sizeof(buff) - 1);
    if (res > 0)
    {
        buff[res] = 0; // 将缓冲区最后一个字节写为空字符，便于后面%s输出
        logMessage(NORMAL, FILENAME, LINE, "get client message from [%d]: %s", server->rdArray[fd], buff);
    }
    else if (res == 0)
    {
        // 对端关闭了连接
        logMessage(NORMAL, FILENAME, LINE, "client[%d] closed, me too\n", server->rdArray[fd]);
        close(server->rdArray[fd]);
        server->rdArray[fd] = FD_NONE;
    }
    else
    {
        logMessage(ERROR, FILENAME, LINE, "read error, close client[%d]\n", server->rdArray[fd]);
        close(server->rdArray[fd]);
        server->rdArray[fd] = FD_NONE;
    }
}

static void eventHandler(fd_set *rfds, struct SelectServer *server)
{
    for (int i = 0; i < NUM; i++)
    {
        if (server->rdArray[i] == FD_NONE)
            continue;
        if (FD_ISSET(server->rdArray[i], rfds))
        {
            if (i == 0)
                accepter(server);
            else
                reader(i, server);
        }
    }
}
static void showFds(struct SelectServer *server)
{
    printf("fds::");
    for (int i = 0; i < NUM; i++)
    {
        if (server->rdArray[i] == FD_NONE)
            continue;
        printf(" %d ", server->rdArray[i]);
    }
    printf("\n");
}
static void accepter(struct SelectServer *server)
{
    char *ip;
    uint16_t clientPort;

    // 从连接队列中取出新连接
    int sock = myAccept(server->listenSock, ip, &clientPort);
    assert(sock >= 0);
    logMessage(NORMAL, FILENAME, LINE, "get link --> client[%s:%d]", ip, clientPort);

    // 在server里的数组里找一个位置放新接受的连接文件描述符
    int pos = 1;
    for (; pos < NUM; pos++)
    {
        if (server->rdArray[pos] == FD_NONE)
            break;
    }
    if (pos == NUM)
    {
        printf("文件描述符集已满，无法接受连接\n");
        close(server->listenSock);
        return;
    }
    else
    {
        printf("new fd::%d\n", sock);
        server->rdArray[pos] = sock;
    }
}