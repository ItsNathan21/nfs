#include "server.h"
#include "../logging/logger.h"
#include "../common/net.h"
#include "../common/alloc.h"
#include "llist.h"
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>

struct leader_arg
{
    int connfd;
};

struct follower_arg
{
};

static timer_t timeout_timer;
static const struct itimerspec leader_alarm = {
    .it_value = {
        .tv_sec = 1L,
        .tv_nsec = 0L,
    },
    .it_interval = {
        .tv_sec = 0L,
        .tv_nsec = 0L,
    }};

static const struct itimerspec follower_alarm = {
    .it_value = {
        .tv_sec = 0L,
        .tv_nsec = 750000000L,
    },
    .it_interval = {
        .tv_sec = 0L,
        .tv_nsec = 0L,
    }};

static struct server *server;

static void new_server(enum server_type_t type, uint32_t port, char *leader_addr, uint32_t leader_port)
{
    server = xmalloc(sizeof(struct server));
    server->leader_addr = xmalloc(sizeof(char) * INET_ADDRSTRLEN);
    server->addr = xmalloc(sizeof(char) * INET_ADDRSTRLEN);
    server->followers = new_dll();
    pthread_mutex_init(&server->mux, NULL);

    server->port = port;
    server->type = type;
    server->leader_port = leader_port;
    server->timer_initialized = 0;

    if (leader_addr != NULL)
    {
        strcpy(server->leader_addr, leader_addr);
    }

    strcpy(server->addr, "127.0.0.1");
}

static void free_msg(struct msg *msg)
{
    free(msg);
}

static void free_server()
{
    free(server->addr);
    free(server->leader_addr);
    if (server->leader_fd > 0)
    {
        close(server->leader_fd);
    }
    free(server);
}

static enum server_type_t get_server_state()
{
    pthread_mutex_lock(&server->mux);
    enum server_type_t state = server->type;
    pthread_mutex_unlock(&server->mux);
    return state;
}

static struct msg *new_ping()
{
    struct msg *msg = xmalloc(sizeof(struct msg));
    msg->type = PING;
    msg->sender_port = server->port;
    msg->sender_addr = (uint8_t *)server->addr;
    msg->addr_len = strlen(server->addr);
    return msg;
}

static void ping_leader()
{
    struct msg *msg = new_ping(server);
    send_msg(msg, server->leader_fd);
    free_msg(msg);
}

static void check_idle_followers()
{
    pthread_mutex_lock(&server->mux);
    for (struct node *node = server->followers->head; node != NULL;)
    {
        struct connected_server *follower = (struct connected_server *)node->data;
        if (!follower->received_ping_in_period)
        {
            struct node *next = node->next;
            remove_dll(server->followers, node);
            node = next;
        }
        else
        {
            follower->received_ping_in_period = 0;
            node = node->next;
        }
    }
    pthread_mutex_unlock(&server->mux);
}

static void timeout_handler(int signum, siginfo_t *info, void *context)
{
    switch (get_server_state())
    {
    case LEADER:
    {
        log_print("Leader got timeout, checking for inactive clients");
        check_idle_followers();
        print_dll(server->followers);
        timer_settime(timeout_timer, 0, &leader_alarm, NULL);
        return;
    }
    case FOLLOWER:
    {
        log_print("Follower got timeout, sending leader a ping");
        ping_leader();
        timer_settime(timeout_timer, 0, &follower_alarm, NULL);
        return;
    }
    case CANDIDATE:
    {
        log_print("Candidate got timeout, idk what the fuck to do yet");
        return;
    }
    }
}

//! server->mux is already locked when entering here
static void init_timeout(const struct itimerspec *alarm_spec)
{
    struct sigaction sigact;
    sigemptyset(&sigact.sa_mask);
    sigact.sa_sigaction = timeout_handler;
    sigact.sa_flags = SA_SIGINFO;

    sigaction(SIGRTMIN + 0, &sigact, NULL);

    struct sigevent sigevt;
    sigevt.sigev_notify = SIGEV_SIGNAL;
    sigevt.sigev_signo = SIGRTMIN + 0;
    sigevt.sigev_value.sival_ptr = NULL;

    if (timer_create(CLOCK_REALTIME, &sigevt, &timeout_timer))
        return;

    if (timer_settime(timeout_timer, 0, alarm_spec, NULL))
        return;

    server->timer_initialized = 1;
}

/*
    Returns the follower associated with ping if known, or
    NULL otherwise
*/
static struct connected_server *follower_already_known(struct msg *msg)
{
    if (server->followers->len <= 0)
    {
        return NULL;
    }
    for (struct node *node = server->followers->head; node != NULL; node = node->next)
    {
        struct connected_server *follower = (struct connected_server *)node->data;
        if ((follower->port == msg->sender_port) &&
            (!strcmp(follower->addr, msg->sender_addr)))
        {
            return follower;
        }
    }
    return NULL;
}

static void handle_msg(int connfd, struct msg *msg)
{
    switch (msg->type)
    {
    case PING:
    {
        log_print("Leader got new PING, handling");
        struct connected_server *found_follower;
        pthread_mutex_lock(&server->mux);
        if ((found_follower = follower_already_known(msg)) != NULL)
        {
            found_follower->received_ping_in_period = 1;
            pthread_mutex_unlock(&server->mux);
            return;
        }
        pthread_mutex_unlock(&server->mux);
        if (server->followers->len >= MAX_FOLLOWERS)
        {
            log_print("Leader can't handle another follower, ignoring");
            return;
        }
        struct connected_server *follower = xmalloc(sizeof(struct connected_server));
        follower->fd = connfd;
        follower->port = msg->sender_port;
        follower->addr = msg->sender_addr;
        follower->received_ping_in_period = 0;
        pthread_mutex_lock(&server->mux);
        add_dll(server->followers, (void *)follower);
        pthread_mutex_unlock(&server->mux);
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
    while (1)
    {
        struct msg *msg = recv_msg(connfd);
        if (msg == NULL)
        {
            return NULL;
        }
        log_print("Leader routine received new message from client at %s:%d, handling", msg->sender_addr, msg->sender_port);
        handle_msg(connfd, msg);
        free_msg(msg);
    }

    free(arg_pack);
    return NULL;
}

void follower_routine(struct follower_arg *arg)
{
    log_print("Follower thread created, sending ping");
    init_timeout(&follower_alarm);
    ping_leader();
    while (1)
    {
    }
}

void server_main(enum server_type_t type, uint32_t port, char *leader_addr, uint32_t leader_port)
{
    new_server(type, port, leader_addr, leader_port);

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
                if (errno == EINTR)
                    continue;
                else
                {
                    log_print("Accept fail");
                    return;
                }
            }
            log_print("Leader accepted new client. Creating new client thread");
            pthread_t thread;
            struct leader_arg *arg = xmalloc(sizeof(struct leader_arg));
            arg->connfd = connfd;
            pthread_create(&thread, NULL, leader_routine, (void *)arg);
            pthread_mutex_lock(&server->mux);
            if (!server->timer_initialized)
            {
                init_timeout(&leader_alarm);
            }
            pthread_mutex_unlock(&server->mux);
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
