#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>

#define MAX_BUF (100)

FILE *_log = NULL;
const char *log_path = "logs/";
const char *log_perms = "w";
char log_prefix[MAX_BUF];

int log_init(char *name, char *hostport)
{
    char path[MAX_BUF];
    sprintf(path, "%s/%s", log_path, name);
    if ((_log = fopen(path, log_perms)) == NULL)
    {
        return -1;
    }
    sprintf(log_prefix, "server@%s>", hostport);
    return 0;
}

void log_flush()
{
    fflush(_log);
}

void log_print(char *fmt, ...)
{
    if (_log == NULL)
    {
        return;
    }

    va_list args;
    va_start(args, fmt);

    fprintf(_log, "%s", log_prefix);
    vfprintf(_log, fmt, args);
    fprintf(_log, "\n");

    fflush(_log);
    va_end(args);
}

void log_free()
{
    fclose(_log);
}