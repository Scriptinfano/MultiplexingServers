#pragma once
#include <assert.h>
#include <poll.h>
#include "socket.h"
#define FD_NONE -1
#define NFDS 5

struct PollServer
{
    __uint16_t port;
    int listenSock;
    struct pollfd *pfd;
    int nfds;
};
void init_pollserver(const char *ip, __uint16_t port, struct PollServer **server);
void showFds_pollserver(struct PollServer *server);
void start_pollserver(struct PollServer *server);
void eventHandler_pollserver(struct PollServer *server);
void accepter_pollserver(struct PollServer *server);
void reader_pollserver(int pos, struct PollServer *server);
void free_pollserver(struct PollServer *server);
