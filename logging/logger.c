#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>

#define MAX_BUF (100)

FILE *log = NULL;
const char *log_path = "logs/";
const char *log_perms = "w";
char log_prefix[MAX_BUF];

int log_init(char *name, char *hostport)
{
    char path[MAX_BUF];
    sprintf(path, "%s/%s", log_path, name);
    if ((log = fopen(path, log_perms)) == NULL)
    {
        return -1;
    }
    sprintf(log_prefix, "server @ %s >", hostport);
    return 0;
}

void log_print(char *fmt, ...)
{
    if (log == NULL)
    {
        return;
    }
    va_list args;
    va_start(args, fmt);
    fprintf(log, "%s%s%s", log_prefix, args, "\n");
    va_end(args);
}