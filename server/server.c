#include "server.h"
#include "../logging/logger.h"
#include "../common/net.h"
#include <stdlib.h>

node_t *_new_node(serv_state_t init_state, int port)
{
    node_t *node;
    if ((node = malloc(sizeof(node_t))) == NULL)
    {
        log_print("Memory allocation fail for a new server\n");
        exit(1);
    }
    node->port = port;
    node->state = init_state;
    return node;
}

void server_main(serv_state_t state, char *leader_addr, int port)
{
    node_t *node = _new_node(state, port);
    switch (state)
    {
    case LEADER:
    {
    }
    case FOLLOWER:
    {
        int leader_fd;
        if ((leader_fd = connect_to_leader(leader_addr, port)) < 0)
        {
            log_print("Follower failed to connect to leader, returning");
            return;
        }

        node->leader_fd = leader_fd;
        if (inform_leader(node) < 0)
            return;
    }
    case CANDIDATE:
    {
        log_print("serve main ended up with candidate role as initial state...returning");
        return;
    }
    }
}