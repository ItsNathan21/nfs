#ifndef NET
#define NET

#include "../server/server.h"
/*
    Starts a server listening on the nodes port using TCP.
    Returns the fd associated with the listening process, or
    -1 on failure
*/
int setup_listen(struct server *server);

/*
Connects to the servers leader address:port, and returns the
fd associated with the connection, or -1 on error
*/
int setup_connect(struct server *server);

/*
    Send a message to the server denoted by fd
*/
void send_msg(struct msg *msg, int fd);

/*
    Receives a message from fd. Will block
    until one comes
*/
struct msg *recv_msg(int fd);

#endif