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

static struct server *new_server(enum server_type_t type, uint32_t port, char *leader_addr, uint32_t leader_port)
{
    struct server *server = xmalloc(sizeof(struct server));
    server->leader_addr = xmalloc(sizeof(char) * INET_ADDRSTRLEN);
    server->addr = xmalloc(sizeof(char) * INET_ADDRSTRLEN);
    server->followers = xmalloc(sizeof(struct connected_server) * MAX_FOLLOWERS);
    pthread_mutex_init(&server->mux, NULL);

    memset(server->followers, 0, sizeof(struct connected_server) * MAX_FOLLOWERS);

    server->port = port;
    server->type = type;
    server->leader_port = leader_port;

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

/*
    Thread for leader to run in the background to deal with one
    specific connection. Should repeatedly listen for messages from
    the client, then handle the message
*/
void *leader_routine(void *arg)
{
    pthread_detach(pthread_self());
    int *connfd_ptr = (int *)arg;
    if (connfd_ptr == NULL)
    {
        log_print("Got a NULL arg for leader_routine(), aborting thread");
    }
    struct msg *msg = recv_msg(*connfd_ptr);
    log_print("Got a message with %d, %s:%d", msg->type, msg->sender_addr, msg->sender_port);
    free(connfd_ptr);
    free_msg(msg);
    return NULL;
}

void server_main(enum server_type_t type, uint32_t port, char *leader_addr, uint32_t leader_port)
{
    struct server *server = new_server(type, port, leader_addr, leader_port);

    switch (type)
    {
    case LEADER:
    {
        int accept_fd = setup_listen(server);
        struct sockaddr_in sockaddr;
        socklen_t socklen = sizeof(sockaddr);
        while (1)
        {
            int *connfd = xmalloc(sizeof(int));
            if ((*connfd = accept(accept_fd, (struct sockaddr *)&sockaddr, &socklen)) < 0)
            {
                log_print("Accept fail");
            }
            pthread_t thread;
            pthread_create(&thread, NULL, leader_routine, (void *)connfd);
        }
        break;
    }
    case FOLLOWER:
    {
        int connfd = setup_connect(server);
        if (connfd < 0)
        {
            break;
        }
        server->leader_fd = connfd;
        struct msg *msg = new_ping(server);
        send_msg(msg, server->leader_fd);
        free_msg(msg);
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