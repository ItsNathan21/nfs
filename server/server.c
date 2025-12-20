#include "server.h"
#include "../logging/logger.h"
#include "../common/net.h"
#include "../common/alloc.h"
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

struct leader_arg
{
    struct server *server;
    int connfd;
};

struct follower_arg
{
    struct server *server;
};

static struct server *new_server(enum server_type_t type, uint32_t port, char *leader_addr, uint32_t leader_port)
{
    struct server *server = xmalloc(sizeof(struct server));
    server->leader_addr = xmalloc(sizeof(char) * INET_ADDRSTRLEN);
    server->addr = xmalloc(sizeof(char) * INET_ADDRSTRLEN);
    server->followers = xmalloc(sizeof(struct connected_server *) * MAX_FOLLOWERS);
    pthread_mutex_init(&server->mux, NULL);

    memset(server->followers, 0, sizeof(struct connected_server *) * MAX_FOLLOWERS);

    server->port = port;
    server->type = type;
    server->leader_port = leader_port;
    server->followers_len = 0U;

    if (leader_addr != NULL)
    {
        strcpy(server->leader_addr, leader_addr);
    }

    strcpy(server->addr, "127.0.0.1");

    return server;
}

static void free_msg(struct msg *msg)
{
    free(msg);
}

static void free_server(struct server *server)
{
    free(server->addr);
    free(server->leader_addr);
    if (server->leader_fd > 0)
    {
        close(server->leader_fd);
    }
    free(server);
}

static struct msg *new_ping(struct server *server)
{
    struct msg *msg = xmalloc(sizeof(struct msg));
    msg->type = PING;
    msg->sender_port = server->port;
    msg->sender_addr = (uint8_t *)server->addr;
    msg->addr_len = strlen(server->addr);
    return msg;
}

static void ping_leader(struct server *server)
{
    struct msg *msg = new_ping(server);
    send_msg(msg, server->leader_fd);
    free_msg(msg);
}

static void handle_msg(struct server *server, int connfd, struct msg *msg)
{
    switch (msg->type)
    {
    case PING:
    {
        log_print("Leader got new PING, handling");
        if (server->followers_len >= MAX_FOLLOWERS)
        {
            log_print("Leader can't handle another follower, ignoring");
            return;
        }
        struct connected_server *follower = xmalloc(sizeof(struct connected_server));
        follower->fd = connfd;
        follower->port = msg->sender_port;
        follower->addr = msg->sender_addr;
        server->followers[server->followers_len++] = follower;
        log_print("Leader added new follower at %s:%d", follower->addr, follower->port);
        return;
    }
    }
}

/*
    Thread for leader to run in the background to deal with one
    specific connection. Should repeatedly listen for messages from
    the client, then handle the message
*/
void *leader_routine(void *arg)
{
    pthread_detach(pthread_self());
    struct leader_arg *arg_pack = (struct leader_arg *)arg;
    if (arg_pack == NULL)
    {
        log_print("Got a NULL arg for leader_routine(), aborting thread");
        return NULL;
    }
    int connfd = arg_pack->connfd;
    struct server *server = arg_pack->server;
    while (1)
    {
        struct msg *msg = recv_msg(connfd);
        if (msg == NULL)
        {
            return NULL;
        }
        log_print("Leader routine received new message from client at %s:%d, handling", msg->sender_addr, msg->sender_port);
        handle_msg(server, connfd, msg);
        free_msg(msg);
    }

    free(arg_pack);
    return NULL;
}

void follower_routine(struct follower_arg *arg)
{
    if (arg == NULL)
    {
        log_print("Follower routine received NULL argument, aborting thread");
        return;
    }
    log_print("Follower thread created, sending ping");
    struct server *server = arg->server;
    ping_leader(server);
    while (1)
    {
    }
}

void server_main(enum server_type_t type, uint32_t port, char *leader_addr, uint32_t leader_port)
{
    struct server *server = new_server(type, port, leader_addr, leader_port);

    switch (type)
    {
    case LEADER:
    {
        log_print("New leader initialized, setting up sockets");
        int accept_fd = setup_listen(server);
        struct sockaddr_in sockaddr;
        socklen_t socklen = sizeof(sockaddr);
        while (1)
        {
            int connfd;
            if ((connfd = accept(accept_fd, (struct sockaddr *)&sockaddr, &socklen)) < 0)
            {
                log_print("Accept fail");
            }
            log_print("Leader accepted new client. Creating new client thread");
            pthread_t thread;
            struct leader_arg *arg = xmalloc(sizeof(struct leader_arg));
            arg->connfd = connfd;
            arg->server = server;
            pthread_create(&thread, NULL, leader_routine, (void *)arg);
        }
        break;
    }
    case FOLLOWER:
    {
        log_print("New follower initialized, settup up connection to leader");
        int connfd = setup_connect(server);
        if (connfd < 0)
        {
            break;
        }
        server->leader_fd = connfd;
        struct follower_arg *arg = xmalloc(sizeof(struct follower_arg));
        arg->server = server;
        follower_routine(arg);
        break;
    }
    case CANDIDATE:
    {
        log_print("Somehow candidate got in as input to server_main(), aborting");
        break;
    }
    }
    free_server(server);
}
