#ifndef LOGGER
#define LOGGER

#define LOG_BUF (200)
/*
    Creates and initializes file for logging with name "name"

    Returns -1 on error, 0 on success
*/
int log_init(char *name, char *hostport);

int log_print(const char *fmt, ...);

void log_flush(void);

void log_free(void);

#endif