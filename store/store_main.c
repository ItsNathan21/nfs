
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include "../main/logger.h"
#include "store_main.h"
#include "../common/common.h"

#define STORE_PORT (12065)

void storer_main()
{
    /*
        Basic vibe:
            Contact other known storers, and compare current files.
            If they have any that we don't, ask them for the full
            data then get it and store it. We should also send the names
            of all of our files, so they can sync with us in case they're
            missing anything we have.

            Once synced, we enter a waiting period of waiting for pushers
            to add new files, then storing it locally, then pushing changes
            to all other storers.
    */

    // initialize socket for communication with other syncers
    int sockfd = setup_socket_listen(STORE_PORT);

    // sync with others
    // will do later :3

    // listen for updates from pushers and other syncers
    while (1)
    {
        struct sockaddr_in addr;
        int len = sizeof(addr);
        int sock;
        if ((sock = accept(sockfd, (struct sockaddr *)&addr, &len)) < 0)
        {
            log_print("Accept failure on socket. Closing storer...");
            return;
        }
        char buf[20];
        recv(sock, buf, sizeof(buf), 0);
        printf("buf is %s\n", buf);
        log_print(buf);
    }
}