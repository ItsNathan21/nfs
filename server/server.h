#ifndef SERVER
#define SERVER

#include <inttypes.h>
#include <pthread.h>

/*
    Current type that this file server is acting as
*/
enum server_type_t
{
    LEADER = 0,
    FOLLOWER = 1,
    CANDIDATE = 2,
};

/*
    The leader needs to know about the follower servers its connected to,
    and this is the metadata the leader has on each one.
*/
struct connected_server
{
    uint32_t port; // port and addr are what they sound like
    uint8_t *addr;
    int fd; // fd to read/write from each follower
};

#define MAX_FOLLOWERS (10)

struct server
{
    uint32_t port;                      // the port this server is running on
    char *addr;                         // our address we're running on
    enum server_type_t type;            // our current type
    int leader_fd;                      // the fd to contact the leader
    char *leader_addr;                  // address of the leader (for connecting)
    uint32_t leader_port;               // port of the leader (for connecting)
    struct connected_server *followers; // array of all followers
    pthread_mutex_t mux;                // mux for locking shared information of this struct
};

enum msg_type
{
    PING = 0,
};

struct msg
{
    uint32_t type;
    uint32_t sender_port;
    uint32_t addr_len;
    uint8_t *sender_addr;
};

/*
    Main routine for the server. Deals with all of the internals, good luck
*/
void server_main(enum server_type_t type, uint32_t port, char *leader_addr, uint32_t leader_port);

#endif