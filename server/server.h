typedef enum
{
    LEADER = 0,
    FOLLOWER = 1,
    CANDIDATE = 2,
} serv_state_t;

void server_main(serv_state_t state, char *leader_addr, int port);