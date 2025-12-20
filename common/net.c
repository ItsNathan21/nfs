#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "alloc.h"
#include "../logging/logger.h"
#include "net.h"

#define MSG_BUF (2000)

int setup_listen(struct server *server)
{
    int sock;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        log_print("Failed to initialize socket");
        return -1;
    }

    struct sockaddr_in sockaddr;
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_addr.s_addr = INADDR_ANY;
    sockaddr.sin_port = htons(server->port);

    if (bind(sock, (struct sockaddr *)&sockaddr, sizeof(sockaddr)) < 0)
    {
        log_print("Failed to bind socket");
        return -1;
    }

    if (listen(sock, 10) < 0)
    {
        log_print("Listen error");
        return -1;
    }

    return sock;
}

int setup_connect(struct server *server)
{
    int sock;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        log_print("Failed to initialize socket");
        return -1;
    }
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server->leader_port);

    if (inet_pton(AF_INET, server->leader_addr, &server_addr.sin_addr) <= 0)
    {
        log_print("inet_pton fail");
        return -1;
    }

    int status;
    if ((status = connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr))) < 0)
    {
        log_print("Connect fail");
        return -1;
    }
    return sock;
}

static int serialize_uint32(uint32_t val, char buf[MSG_BUF], int idx)
{
    buf[idx++] = (uint8_t)((val >> 24) & 0xff);
    buf[idx++] = (uint8_t)((val >> 16) & 0xff);
    buf[idx++] = (uint8_t)((val >> 8) & 0xff);
    buf[idx++] = (uint8_t)((val >> 0) & 0xff);
    return idx;
}

static int serialize_msg(struct msg *msg, char buf[MSG_BUF])
{
    int idx = 0UL;

    idx = serialize_uint32(msg->type, buf, idx);
    idx = serialize_uint32(msg->sender_port, buf, idx);
    idx = serialize_uint32(msg->addr_len, buf, idx);

    strcpy(buf + idx, msg->sender_addr);
    idx += msg->addr_len;

    return idx;
}

static int deserialize_uint32(uint32_t *val, char buf[MSG_BUF], int idx)
{
    *val = ((uint32_t)(uint8_t)buf[idx] << 24) |
           ((uint32_t)(uint8_t)buf[idx + 1] << 16) |
           ((uint32_t)(uint8_t)buf[idx + 2] << 8) |
           ((uint32_t)(uint8_t)buf[idx + 3]);

    idx += 4;

    return idx;
}

static void deserialize_msg(struct msg *msg, char buf[MSG_BUF])
{
    memset(msg, 0, sizeof(struct msg));

    int idx = 0;

    idx = deserialize_uint32(&msg->type, buf, idx);
    idx = deserialize_uint32(&msg->sender_port, buf, idx);
    idx = deserialize_uint32(&msg->addr_len, buf, idx);

    msg->sender_addr = xmalloc(sizeof(uint8_t) * msg->addr_len);

    strncpy(msg->sender_addr, buf + idx, msg->addr_len);
    msg->sender_addr[msg->addr_len] = '\0';
    idx += msg->addr_len;
}

static ssize_t write_all(int fd, const void *buf, size_t len)
{
    size_t total = 0;
    const char *ptr = buf;
    while (total < len)
    {
        ssize_t n = write(fd, ptr + total, len - total);
        if (n <= 0)
            return -1;
        total += n;
    }
    return total;
}

ssize_t read_all(int fd, void *buf, size_t len)
{
    size_t total = 0;
    char *ptr = buf;
    while (total < len)
    {
        ssize_t n = read(fd, ptr + total, len - total);
        if (n <= 0)
            return -1;
        total += n;
    }
    return total;
}

void send_msg(struct msg *msg, int fd)
{
    char buf[MSG_BUF];
    int len = serialize_msg(msg, buf);

    uint32_t net_len = htonl(len);

    write_all(fd, &net_len, sizeof(net_len));
    write_all(fd, buf, len);
}

struct msg *recv_msg(int fd)
{
    uint32_t net_len;
    if (read_all(fd, &net_len, sizeof(net_len)) < 0)
    {
        log_print("Error reading message length");
        return NULL;
    }

    uint32_t len = ntohl(net_len);
    if (len > MSG_BUF)
    {
        log_print("Message too big");
        return NULL;
    }

    char buf[MSG_BUF];
    if (read_all(fd, buf, len) < 0)
    {
        log_print("Error reading message body");
        return NULL;
    }

    struct msg *msg = xmalloc(sizeof(struct msg));
    deserialize_msg(msg, buf);
    return msg;
}