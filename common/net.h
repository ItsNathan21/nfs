#include "../server/server.h"'

/*
Maximum buffer needed for communicating between servers
*/
#define COMM_BUF (1000)
#define ADDR_LEN (25)
/*
    Binds a socket to leader_addr, returning that file
    descriptor to communicate with the leader. Returns -1 on fail

    Uses port specifified by port to connect to leader with
*/
int connect_to_leader(char *leader_addr, int port);

/*
    Informs the leader about our nodes existence, so it can communicate with us.
    The node struct should be fully initialized by this point

    Returns -1 on fail, 0 on success
*/
int inform_leader(node_t *node);