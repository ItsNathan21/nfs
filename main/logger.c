#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include "logger.h"

const char *_log_path = "logs/log.txt";
const char *_log_prefix = "> ";
FILE *_log_file = NULL;
bool _initialized = false;

void _init_logger()
{
    _log_file = fopen(_log_path, "a");
    if (_log_file == NULL)
    {
        fprintf(stderr, "Failed to open file at %s\n", _log_path);
        return;
    }
    _initialized = true;
}

void log_print(char msg[LOG_BUF])
{
    if (!_initialized)
        _init_logger();
    time_t t = time(NULL);
    char *curr_time = asctime(localtime(&t));
    fprintf(_log_file, "Server@%s%s%s", curr_time, _log_prefix, msg);
}
