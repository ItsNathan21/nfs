#include <inttypes.h>

typedef enum
{
    LEADER = 0,
    FOLLOWER = 1,
    CANDIDATE = 2,
} serv_state_t;

typedef struct
{
    int leader_fd;      // fd to read/write to/from the leader
    int port;           // the port we're running our server off of
    serv_state_t state; // our current state
} node_t;

typedef enum
{
    PING = 0
} msg_type_t;

typedef struct
{
    uint32_t type;     // type of the message being sent (a msg_type_t)
    char *sender_addr; // address (host:port) of the sender, so we can connect with the sender if needed
} msg_t;

void server_main(serv_state_t state, char *leader_addr, int port);