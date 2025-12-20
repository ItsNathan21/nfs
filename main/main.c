#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../server/server.h"
#include "../logging/logger.h"

#define NAMEBUF (100)

void usage()
{
    printf("usage is :\n");
    printf("./nfs_sys {follower, leader} port {leader addr} {leader port}\n");
}

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        usage();
        return EXIT_FAILURE;
    }

    int port = atoi(argv[2]);

    char name[NAMEBUF];
    if (!strcmp(argv[1], "follower"))
    {
        if (argc < 4)
        {
            usage();
            return EXIT_FAILURE;
        }

        sprintf(name, "Follower@%d", atoi(argv[2]));
        log_init(name, argv[2]);
        char *leader_addr = argv[3];
        int leader_port = atoi(argv[4]);
        server_main(FOLLOWER, port, leader_addr, leader_port);
    }
    else if (!strcmp(argv[1], "leader"))
    {
        sprintf(name, "Leader@%d", atoi(argv[2]));

        log_init(name, argv[2]);
        server_main(LEADER, port, NULL, -1);
    }

    log_free();
    return EXIT_SUCCESS;
}