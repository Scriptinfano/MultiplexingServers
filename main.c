#include "select_server.h"
#include "poll_server.h"
#include "epoll_server.h"
#include "tool.h"
void test_selectserver()
{
    struct SelectServer *p = initSelectServer(8080, "127.0.0.1");
    start(p);
}
void test()
{
    SetNonBlock(0);
    while (1)
    {
        char buff[128];
        ssize_t s = read(0, buff, sizeof(buff) - 1); // 读127字节是为了给最后一个位置留下空字符，以达到字符串输出要求
        if (s > 0)
        {
            buff[s - 1] = 0; // 去掉读到的换行符
            printf("%s\n", buff);
        }
        else
        {
            sleep(1);
            if (errno == EWOULDBLOCK)
            {
                printf("current resource not ready temporarily\n");
                continue;
            }
            else if (errno == EINTR)
            {
                printf("current io interupted by signal");
                continue;
            }
            else
            {

                printf("read error\n");
                break;
            }
        }
    }
}
void test_pollserver()
{
    struct PollServer *server = NULL; // 所有结构体指针声明之后最好初始化为空
    init_pollserver("127.0.0.1", 12345, &server);
    start_pollserver(server);
    free_pollserver(server);
}
void test_epollserver()
{
    struct EpollServer *server = NULL;
    init_epollserver("127.0.0.1", 12345, &server);
    start_epollserver(server);
    free_epollserver(server);
}
int main()
{
    test();
}