#include <stdlib.h>
#include "../logging/logger.h"
#include "alloc.h"

void *xmalloc(size_t size)
{
    void *ptr;
    if ((ptr = malloc(size)) == NULL)
    {
        log_print("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }
    return ptr;
}