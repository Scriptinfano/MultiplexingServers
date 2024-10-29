#include "epoll_server.h"
#include "log.h"
#include "tool.h"
#include "socket.h"
#include <unistd.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
int initEpollModel()
{
    int epfd = epoll_create1(EPOLL_CLOEXEC); // 在执行exec族替换当前进程的时候自动关闭当前进程仍然处于打开状态的epfd
    if (epfd == -1)
    {
        logMessage(FATAL, FILENAME, LINE, "create epoll fail,errno[%d]::%s", errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
    return epfd;
}
Connection *createConnection(int sock, EpollServer *prsvr){
    Connection *con=(Connection*)malloc(sizeof(Connection));
    if(con==NULL){
        logMessage(FATAL,LOGPARAM,"无法为connection分配内存");
        exit(EXIT_FAILURE);
    }
    con->except_callback=NULL;
    con->read_callback=NULL;
    con->send_callback=NULL;
    memset(con->recv_buf, 0, sizeof(con->recv_buf)); // 将 recv_buf 清零
    memset(con->send_buf, 0, sizeof(con->send_buf)); // 将 send_buf 清零
    con->sock=sock;
    con->prsvr = prsvr;
    return con;
}
/*
@brief 初始化epollServer
*/
EpollServer *initEpollServer(const char *ip, uint16_t port)
{
    EpollServer *server = (EpollServer *)malloc(sizeof(EpollServer));
    if (server != NULL)
    {
        server->listenSock = mySocket();
        server->port = port;
        server->revents = (struct epoll_event *)malloc(sizeof(struct epoll_event) * MAX_EVENTS);
        server->maxRevents = MAX_EVENTS;
        myBind(server->listenSock, port, ip);
        myListen(server->listenSock);
        server->epfd = initEpollModel();
        server->sock_con_table = createHashTable();
        // 由于listenSock只需要关心读事件，也不需要读写缓冲区。所以不再为其设置回调函数
        controlEpoll(EPOLL_CTL_ADD, server->listenSock, EPOLLIN, NULL, server);
    }
    else
    {
        logMessage(FATAL, FILENAME, LINE, "PollServer无法被分配内存");
        exit(EXIT_FAILURE);
    }
    logMessage(DEBUG, FILENAME, LINE, "init epoll server complete!");
    return server;
}
/*
@brief 开始运行EpollServer
@param timeout 设置EPollServer在调用epoll_wait的时候的超时时间
@param server 调用initEpollServer得到的EpollSerer地址
*/
void startEpollServer(int timeout,EpollServer *server)
{
    while (1)
    {
        int res = waitEpoll(timeout,server);//内部已经处理了返回值为-1的情况，现在只需要处理为0和大于0的情况
        if (res == 0)
        {
            //超时之后重新调用
            logMessage(NORMAL, FILENAME, LINE, "epoll wait time out, wait again...");
        }
        else
        {
            //拿到了所有就绪事件，准备在server->revents中处理所有就绪的事件
            logMessage(NORMAL, FILENAME, LINE, "get ready events from epoll model");
            handleEpollServerEvent(res, server);
        }
    }
}


/*
@brief 对epoll模型执行增删改操作，如果是增操作，则需要设置第四个参数，指定三个回调函数组成的数组的地址，数组必须也只能有三个元素分别存放读、写、异常回调函数的地址
@param op 要执行哪一种类型的操作，EPOLL_CTL_
@param fd 对哪一个文件描述符上的事件感兴趣
@param events 对什么类型的事件感兴趣
@param callback_arr 回调函数指针组成的数组，三个元素分别是读回调、写回调、异常回调，如果这个参数是空，那说明调用的时候暂时不需要设置回调函数，在后面用其他函数设置
*/
void controlEpoll(int op, int fd, uint32_t events,func_t *callback_arr,EpollServer*server)
{
    struct epoll_event event;
    event.events = events;
    event.data.fd = fd; // data中有四个都可表示文件描述符
    if(op==EPOLL_CTL_ADD &&callback_arr!=NULL){
        //如果是要添加文件描述符，该函数要考虑到为文件描述符设置相应回调函数的情况
        Connection *con_ptr = createConnection(fd, server);
        con_ptr->read_callback = callback_arr[0];
        con_ptr->send_callback = callback_arr[1];
        con_ptr->except_callback = callback_arr[2];
        //不要忘了将connection加入到EpollServer的哈希表中
        insertHashTable(server->sock_con_table, intToString(fd), con_ptr);
    }
    int res = epoll_ctl(server->epfd, op, fd, &event);
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
int waitEpoll(int timeout,EpollServer *server)
{
    int res = epoll_wait(server->epfd, server->revents, server->maxRevents, timeout);
    if (res == -1)
    {
        logMessage(FATAL, FILENAME, LINE, "epoll_waot fail,errno[%d]::%s", errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
    return res;
}
void acceptEpollServer(EpollServer *server)
{
    char clientIp[16] = {'\0'};
    uint16_t clientPort;
    
    int sock = myAccept(server->listenSock, clientIp, &clientPort);
    logMessage(DEBUG, FILENAME, LINE, "new client socket accetped[%s:%d]", clientIp, clientPort);
    if (sock > 0)
    {
        //TODO 在这里传入连接套接字的三个回调函数
        controlEpoll(EPOLL_CTL_ADD, sock, EPOLLIN,,server);//将新接收的连接描述符也放到epoll模型中监管，以便该连接的数据就绪时能被放入就绪队列，这样我们下次调用epoll_wait的时候就能取得就绪的描述符
    }
}
void readEpollServer(int sock, EpollServer *server)
{
    char buf[128] = {'\0'};
    //TODO 在这里做连接套接字就绪之后的回调函数调用，server->sock_con_table
    int res = read(sock, buf, sizeof(buf));
    if (res > 0)
    {
        // SUCCESS
        logMessage(NORMAL, FILENAME, LINE, "client message:%s", buf);
    }
    else if (res == 0)
    {
        // EOF 关闭文件描述符，注意这里在关闭文件描述符的时候，必须先让epoll模型取消对sock的关心，然后再close掉sock
        logMessage(NORMAL, FILENAME, LINE, "client[%d] closed, me too", sock);
        controlEpoll(server->epfd, EPOLL_CTL_DEL, sock, EPOLLIN,NULL);
        close(sock);
    }
    else
    {
        // read error
        logMessage(ERROR, FILENAME, LINE, "client[%d] read error,errno[%d]::%s", sock, errno, strerror(errno));
        // 先让epoll取消对指定套接字上某事件的关心，然后再去关闭socket
        controlEpoll(server->epfd, EPOLL_CTL_DEL, sock, EPOLLIN,NULL);
        close(sock);
    }
}
/*
@param n epoll_event数组的前n个代表就绪的事件，一般从epoll_wait的返回值得到
@param server EpollServer的地址
*/
void handleEpollServerEvent(int n, EpollServer *server)
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
                acceptEpollServer(server);
            }
            else
            {
                //TODO 如果是连接的要读取数据的套接字就绪了，则为其调用设置好的读回调函数
                readEpollServer(sock, server);
            }
        }
        if(events & EPOLLOUT){
            //TODO 写事件就绪，只有连接套接字才可能写事件就绪，这里不需要考虑监听套接字的情况
        }
        if(events & EPOLLERR){
            //TODO 异常事件发生，根据套接字的类型调用相应的回调处理函数
        }
    }
}

/*
@brief EpollServer的析构函数
@param server EpollServer地址
*/
void freeEpollServer(EpollServer *server)
{
    if (server->listenSock >= 0)
        close(server->listenSock);
    if (server->revents != NULL)
        free(server->revents);
    if (server->epfd >= 0)
        close(server->epfd);
    logMessage(DEBUG, FILENAME, LINE, "epoll has been successfully freed");
}