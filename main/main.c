#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../server/server.h"

void usage()
{
    printf("Usage is ./nfs server {LEADER, FOLLOWER} portNum <leader address (for FOLLOWER only)>\n");
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        usage();
        return 1;
    }

    if (!strcmp(argv[1], "server"))
    {
        if (argc < 4)
        {
            printf("Need to provide a state of leader or follower to the server + port number\n");
            return 1;
        }
        serv_state_t state;
        if (!strcmp(argv[2], "leader"))
        {
            state = LEADER;
            server_main(LEADER, NULL, atoi(argv[3]));
        }
        else
        {
            if (argc < 5)
            {
                printf("Need to provide a leaders address for the follower to contact\n");
            }
            state = FOLLOWER;
            server_main(FOLLOWER, argv[4], atoi(argv[3]));
        }
    }

    return 0;
}