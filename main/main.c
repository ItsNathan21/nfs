#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../server/server.h"
#include "../logging/logger.h"

void usage()
{
    printf("Usage is ./nfs server {leader, follower} portNum <leader address (for FOLLOWER only)>\n");
}

int main(int argc, char **argv)
{
    if (argc < 3)
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

        char hostport[100], name[100];

        sprintf(hostport, "127.0.0.1/%s", argv[3]);
        sprintf(name, "server@%s", argv[3]);

        if (log_init(name, hostport) < 0)
        {
            printf("Failed to initialize logging at with name %s\n", name);
            return 1;
        }

        serv_state_t state;
        if (!strcmp(argv[2], "leader"))
        {
            state = LEADER;
            server_main(LEADER, NULL, atoi(argv[3]));
        }
        else if (!strcmp(argv[2], "follower"))
        {
            if (argc < 5)
            {
                printf("Need to provide a leaders address for the follower to contact\n");
            }
            state = FOLLOWER;
            server_main(FOLLOWER, argv[4], atoi(argv[3]));
        }
        else
        {
            usage();
            return 1;
        }
    }

    return 0;
}