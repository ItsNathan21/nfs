#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "logger.h"
#include <time.h>
#include "../store/store_main.h"
#include "../push/push_main.h"

/*
    Options for the server to act as.

    STORER : Runs perpetually in the background, syncing with
        other storers.
    PULLER: Temporarily runs, just sync's current files with
        those determined by the running storers. If no
        currently running storers, it fails to sync,
        and will return.
    PUSHER: Wants to push new files to the storers, syncing with
        new ones if necessary, and dealing with conflicts
        with a LWW rule
*/

void usage()
{
    printf("usage of server is:\n ./server <type>\n <type> store, pull, push\n");
}

int main(int argc, char **argv)
{
    log_print("Program beginning");
    char *op;

    if (argc < 2)
    {
        usage();
        return EXIT_FAILURE;
    }

    if (strcmp(argv[1], "store") == 0)
    {
        op = "store";
        storer_main();
    }
    else if (strcmp(argv[1], "pull") == 0)
    {
        op = "pull";
    }
    else if (strcmp(argv[1], "push") == 0)
    {
        op = "push";
        pusher_main();
    }
    else
    {
        op = "unknown";
        printf("Unknown option %s\n", argv[1]);
    }

    char log[LOG_BUF];
    sprintf(log, "Returning from main's, closing %s operation now\n", op);
    log_print(log);

    return EXIT_SUCCESS;
}