#ifndef LLIST

#define LLIST

#include <inttypes.h>

struct node
{
    void *data;
    struct node *prev;
    struct node *next;
};

struct dll
{
    struct node *head;
    struct node *tail;
    uint32_t len;
};

struct dll *new_dll();
/*
    None of these functions are thread safe
*/
void add_dll(struct dll *dll, void *data);
void remove_dll(struct dll *dll, struct node *node);
void print_dll(struct dll *dll);

#endif
