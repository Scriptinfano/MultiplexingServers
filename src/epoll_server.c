#include "epoll_server.h"
/*
初始化epollServer
*/
void init_epollserver(const char *ip, uint16_t port, struct EpollServer **server)
{
    *server = (struct EpollServer *)malloc(sizeof(struct EpollServer));
    if ((*server) != NULL)
    {
        (*server)->listenSock = mySocket();
        (*server)->port = port;
        (*server)->revents = (struct epoll_event *)malloc(sizeof(struct epoll_event) * MAX_EVENTS);
        (*server)->maxRevents = MAX_EVENTS;
        myBind((*server)->listenSock, port, ip);
        myListen((*server)->listenSock);
        (*server)->epfd = init_epollmodel();
        ctl_epoll((*server)->epfd, EPOLL_CTL_ADD, (*server)->listenSock, EPOLLIN);
    }
    else
    {
        logMessage(FATAL, FILENAME, LINE, "PollServer无法被分配内存");
        exit(EXIT_FAILURE);
    }
    logMessage(DEBUG, FILENAME, LINE, "init epoll server complete!");
}
/*
开始运行EpollServer
*/
void start_epollserver(struct EpollServer *server)
{
    int timeout = 2000;
    while (1)
    {
        int res = wait_epoll(server->epfd, server->revents, server->maxRevents, timeout);
        if (res == 0)
        {
            logMessage(NORMAL, FILENAME, LINE, "epoll wait time out, wait again...");
        }
        else
        {
            logMessage(NORMAL, FILENAME, LINE, "get a link");
            handler_epollserver(res, server);
        }
    }
}
/*
EpollServer的析构函数
*/
void free_epollserver(struct EpollServer *server)
{
    if (server->listenSock >= 0)
        close(server->listenSock);
    if (server->revents != NULL)
        free(server->revents);
    if (server->epfd >= 0)
        close(server->epfd);
    logMessage(DEBUG, FILENAME, LINE, "epoll has been successfully freed");
}
int init_epollmodel()
{
    int res = epoll_create1(EPOLL_CLOEXEC);
    if (res == -1)
    {
        logMessage(FATAL, FILENAME, LINE, "create epoll fail,errno[%d]::%s", errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
    return res;
}
void ctl_epoll(int epfd, int op, int fd, uint32_t events)
{
    struct epoll_event event;
    event.events = events;
    event.data.fd = fd; // data中有四个都可表示文件描述符
    int res = epoll_ctl(epfd, op, fd, &event);
    if (res == -1)
    {
        logMessage(FATAL, FILENAME, LINE, "epoll_ctl fail errno[%d]::%s", errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
}
/*
返回就绪事件的个数
epoll这里底层会将就绪的n个事件全部放在我们传进去的events数组
的前n位，比如说我们传进去的数组总共10个元素，当底层有4个事
件就绪的时候会将四个事件全部放在数组的前四位，对应下标就是
0、1、2、3这四个位，返回值为4，所以我们上层想要找就绪时间的
时候只需要遍历数组的0 ~ n位，而不是像select和poll那样将整个
数组都遍历一边，所以epoll每次处理时间的时候效率都会很高
*/
int wait_epoll(int epfd, struct epoll_event *events, int maxevents, int timeout)
{
    int res = epoll_wait(epfd, events, maxevents, timeout);
    if (res == -1)
    {
        logMessage(FATAL, FILENAME, LINE, "epoll_waot fail,errno[%d]::%s", errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
    return res;
}
void accepter_epollserver(struct EpollServer *server)
{
    char clientIp[16] = {'\0'};
    uint16_t clientPort;
    int sock = myAccept(server->listenSock, clientIp, &clientPort);
    logMessage(DEBUG, FILENAME, LINE, "new client socket accetped[%s:%d]", clientIp, clientPort);
    if (sock > 0)
    {
        ctl_epoll(server->epfd, EPOLL_CTL_ADD, sock, EPOLLIN);
    }
}
void reader_epollserver(int sock, struct EpollServer *server)
{
    char buf[128] = {'\0'};
    int res = read(sock, buf, sizeof(buf));
    if (res > 0)
    {
        // SUCCESS
        logMessage(NORMAL, FILENAME, LINE, "client message:%s", buf);
    }
    else if (res == 0)
    {
        // EOF 关闭文件描述符
        logMessage(NORMAL, FILENAME, LINE, "client[%d] closed, me too", sock);
        ctl_epoll(server->epfd, EPOLL_CTL_DEL, sock, EPOLLIN);
        close(sock);
    }
    else
    {
        // read error
        logMessage(ERROR, FILENAME, LINE, "client[%d] read error,errno[%d]::%s", sock, errno, strerror(errno));
        // 先让epoll取消对指定套接字上某事件的关心，然后再去关闭socket
        ctl_epoll(server->epfd, EPOLL_CTL_DEL, sock, EPOLLIN);
        close(sock);
    }
}
/*
参数：
    int n: epoll_event数组的前n个代表就绪的事件，一般从epoll_wait的返回值得到
    struct EpollServer *server:
*/
void handler_epollserver(int n, struct EpollServer *server)
{
    // epoll的好处就在于调用epoll_wait的时候从其输出型参数所代表的epoll_event数组中就能快速得到哪些事件就绪了，就是数组的前n个元素，n就是epoll_wait成功之后的返回值
    for (int i = 0; i < n; i++)
    {
        // 拿出就绪的事件
        uint32_t events = server->revents[i].events; // 这个整型转换为二进制后某些位就代表具体的IO事件
        int sock = server->revents[i].data.fd;       // 哪个文件描述符代表的socket其事件就绪了
        if (events & EPOLLIN)
        {
            // 读事件就绪
            if (sock == server->listenSock)
            {
                // 如果是监听套接字的读事件就绪
                accepter_epollserver(server);
            }
            else
            {
                // 如果是连接的要读取数据的套接子，则读取
                reader_epollserver(sock, server);
            }
        }
    }
}