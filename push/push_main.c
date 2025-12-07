#include "../common/common.h"
#include "../main/logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

void pusher_main()
{
    /*
        find a syncer, push to them then push the data
    */
    int sockfd = setup_socket_connect("127.0.0.1", 12065);
    if (sockfd < 0)
    {
        log_print("Failed to setup connection to store\n");
        return;
    }
    char buf[] = "Hello from pusher!";
    if (send(sockfd, buf, sizeof(buf), 0) < 0)
    {
        log_print("Failed to push message to storer\n");
    }
    close(sockfd);
}