#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "../logging/logger.h"
#include "net.h"

int connect_to_leader(char *leader_addr, int port)
{
    int sock;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        log_print("Failed to create TCP socket\n");
        return -1;
    }
    struct sockaddr_in sockaddr;
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = port;
    if (inet_pton(AF_INET, "127.0.0.1", &sockaddr.sin_addr) <= 0)
    {
        log_print("Failed to resolve localhost to a valid address");
        return -1;
    }

    int status;
    if ((status = connect(sock, (struct sockaddr *)&sockaddr, sizeof(sockaddr))) < 0)
    {
        log_print("Failed to connect to leader @ %s", leader_addr);
        return -1;
    }
    return sock;
}

/*
    Creates a new message for a given node (might use some node metadata)
*/
msg_t *_new_msg(node_t *node, msg_type_t type)
{
    msg_t *msg;
    if ((msg = malloc(sizeof(msg_t))) == NULL)
    {
        log_print("Memory allocation failed for a msg");
        exit(1);
    }
    if ((msg->sender_addr = malloc(sizeof(char) * ADDR_LEN)) == NULL)
    {
        log_print("Memory allocation failed for sender address");
        exit(1);
    }
    sprintf(msg->sender_addr, "127.0.0.1:%d", node->port);
    return msg;
}

/*
    Serializes a msg into dest buffer

    Returns the length of the buffer
*/
int _serialize_msg(char dest[COMM_BUF], msg_t *msg)
{
    uint32_t idx = 0;
    dest[idx++] = (msg->type >> 24) & (0xff);
    dest[idx++] = (msg->type >> 16) & (0xff);
    dest[idx++] = (msg->type >> 8) & (0xff);
    dest[idx++] = (msg->type >> 0) & (0xff);

    int len = strlen(msg->sender_addr);
    strncpy(dest + idx, msg->sender_addr, len);
    idx += len;

    return idx;
}

int inform_leader(node_t *node)
{
    if (node->state != FOLLOWER)
    {
        log_print("Node is not in follower state when trying to inform leader");
        return -1;
    }

    msg_t *msg = _new_msg(node, PING);

    char buf[COMM_BUF];
    int buf_len = _serialize_msg(buf, msg);

    if (write(node->leader_fd, buf, buf_len) < 0)
    {
        log_print("Failed on write to leader during inform stage");
        return -1;
    }

    return 0;
}