#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "../main/logger.h"
#include "common.h"
#include <inttypes.h>

#define LOCALHOST ("127.0.0.1")
#define BACKLOG_SIZE (10)

int setup_socket_listen(int port)
{

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    // setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, NULL, 0);
    struct sockaddr_in sock_in;
    sock_in.sin_family = AF_INET; // IPv4
    sock_in.sin_port = port;      // use the port number specified
    // get out localhost address into binary form to use for the address
    if (inet_pton(AF_INET, LOCALHOST, &sock_in.sin_addr) <= 0)
    {
        char log[LOG_BUF];
        sprintf(log, "Failed to connect to port %d\n", port);
        log_print(log);
        return -1;
    }
    if (bind(sockfd, (struct sockaddr *)&sock_in, sizeof(sock_in)) < 0)
    {
        char log[LOG_BUF];
        sprintf(log, "Failed to bind socket to port %d\n", port);
        log_print(log);
        return -1;
    }
    if (listen(sockfd, BACKLOG_SIZE) < 0)
    {
        char log[LOG_BUF];
        sprintf(log, "listen failed on port %d\n", port);
        log_print(log);
        return -1;
    }

    return sockfd;
}

int setup_socket_connect(const char *server_addr, int server_port)
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in sock_in;
    sock_in.sin_family = AF_INET;
    sock_in.sin_port = server_port;
    if (inet_pton(AF_INET, server_addr, &sock_in.sin_addr) <= 0)
    {
        char log[LOG_BUF];
        sprintf(log, "Failed to parse server address of %s:%d", server_addr, server_port);
        log_print(log);
        return -1;
    }
    if (connect(sockfd, (struct sockaddr *)&sock_in, sizeof(sock_in)) < 0)
    {
        char log[LOG_BUF];
        sprintf(log, "Failed to connect to server %s:%d", server_addr, server_port);
        log_print(log);
        return -1;
    }
    return sockfd;
}

void _add_long(uint32_t l, char buf[SERIALIZE_BUFLEN], int *start_idx)
{
    buf[(*start_idx)++] = (l >> 24) & 0xff;
    buf[(*start_idx)++] = (l >> 16) & 0xff;
    buf[(*start_idx)++] = (l >> 8) & 0xff;
    buf[(*start_idx)++] = (l >> 0) & 0xff;
}

int serialize_s2s(s2s_t *msg, char buf[SERIALIZE_BUFLEN])
{
    int idx = 0;
    uint32_t type = htonl(msg->type);
    _add_long(type, buf, &idx);

    return idx;
}