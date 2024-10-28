#include "socket.h"
#include <strings.h>
static const int gBackLog = 20; // 将变量限定在本文件之内，且不可修改
int mySocket()
{
    int listenSock = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSock == -1)
    {
        logMessage(FATAL, FILENAME, LINE, "server create socket failed");
        exit(EXIT_FAILURE);
    }
    int opt = 1;
    /*
    int setsockopt(int socket, int level, int option_name, const void *option_value, socklen_t option_len);
    socket：需要设置选项的套接字描述符。
    level：选项所在的协议层。例如，SOL_SOCKET 表示套接字层。
    option_name：需要设置的选项名称。例如，SO_REUSEADDR
    option_value：一个指向包含新选项值的缓冲区的指针。
    option_len：option_value 缓冲区的大小，以字节为单位。
    */
    if (setsockopt(listenSock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        logMessage(ERROR, FILENAME, LINE, "setsock failed after create socket");
    }
    logMessage(DEBUG, FILENAME, LINE, "server create socket succes, file descripter on %d", listenSock);
    return listenSock;
}
void myBind(int listenSock, uint16_t port, const char *ip)
{
    struct sockaddr_in addr_in;
    bzero(&addr_in, sizeof(addr_in));          // 建议使用bzero在初始化结构体之前来清空结构体，而不是memset函数，这个更简单
    inet_pton(AF_INET, ip, &addr_in.sin_addr); // 将点分十进制的IPV4地址字符串转换为网络字节序的二进制形式
    addr_in.sin_port = htons(port);            // 将一个16位无符号整数（如端口号）从主机字节顺序转换为网络字节顺序。网络通信中使用的字节顺序是大端序（高位字节存储在低地址）
    addr_in.sin_family = AF_INET;

    struct sockaddr *addr = (struct sockaddr *)&addr_in;

    if (bind(listenSock, addr, sizeof(addr_in)) < 0)
    {
        logMessage(FATAL, FILENAME, LINE, "server bind %s:%d fail, %s", ip, port, strerror(errno));
        exit(EXIT_FAILURE);
    }
    logMessage(DEBUG, FILENAME, LINE, "server bind %s:%d success", ip, port);
}

void myListen(int listenSock)
{
    if (listen(listenSock, gBackLog) < 0)
    {
        logMessage(FATAL, FILENAME, LINE, "server listen fail");
        exit(EXIT_FAILURE);
    }
    logMessage(NORMAL, FILENAME, LINE, "server init success");
}

/*
int listenSock: 监听套接字的文件描述符
char *clientIP：输出型参数，获取连接之后的对端IP
uint16_t *clientPort：输出型参数，获取连接之后的对端端口
返回-1表示发生了错误，返回一个大于0的整数表示成功，且该整数就是新socket的文件描述符
*/
int myAccept(int listenSock, char *clientIP, uint16_t * const clientPort)
{
    struct sockaddr_in client;
    socklen_t clientLen = sizeof(client);
    memset(&client, 0, clientLen);

    int serverSock = accept(listenSock, (struct sockaddr *)&client, &clientLen);
    if (serverSock < 0)
    {
        logMessage(ERROR, FILENAME, LINE, "server accept connection fail");
        return -1;
    }
    char temp[16];
    inet_ntop(AF_INET, &client.sin_addr, temp,sizeof(temp));
    clientIP = (char *)malloc(16);//为输出型参数申请空间
    memcpy(clientIP, temp, 16);//将刚才的temp上的内容复制到输出型参数指向的内存空间中
    *clientPort = ntohs(client.sin_port);
    logMessage(NORMAL, FILENAME, LINE, "server accept connection success: [%s:%d] server sock::%d", clientIP, *clientPort, serverSock);
    return serverSock;
}