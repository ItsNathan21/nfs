#define SERIALIZE_BUFLEN (100)

typedef enum
{
    PING = 0U
} s2s_msg_t;

/*
    Struct to send between storers
*/
typedef struct
{
    s2s_msg_t type;
} s2s_t;

/*
    Struct to send from pusher -> storer
*/
typedef struct
{

} p2s_t;

/*
    Struct to send from storer -> puller
*/
typedef struct
{

} s2p_t;

/*
Sets up a listening socket on LOCALHOST:port using IPv4. Returns
-1 on failure, and the file descriptor associated with the socket
if successful
*/
int setup_socket_listen(int port);

/*
Sets up a socket on LOCALHOST:port using IPv4. Connects to server_addr
at server_port, and returns -1 on any error, or the corresponding fd
with that connection.
*/
int setup_socket_connect(const char *server_addr, int server_port);

/*
    Serializes message for the s2s_t struct into buf, returning
    the length of the buffer.
*/
int serialize_s2s(s2s_t *msg, char buf[SERIALIZE_BUFLEN]);