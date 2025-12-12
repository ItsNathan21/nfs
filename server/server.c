#include "server.h"

void server_main(serv_state_t state, char *leader_addr, int port)
{
    switch (state)
    {
    case LEADER:
    {
    }
    case FOLLOWER:
    {
    }
    case CANDIDATE:
    {
        // log an error because it should not be this
    }
    }
}