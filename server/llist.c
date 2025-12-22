#include "llist.h"
#include "../common/alloc.h"
#include <inttypes.h>
#include <stdio.h>

/*
    Generic linked list node
    Meant to include your own struct in the data
    pointer, then the prev and next are dealt with
*/

struct dll *new_dll()
{
    struct dll *dll = xmalloc(sizeof(struct dll));
    dll->head = NULL;
    dll->tail = NULL;
    dll->len = 0U;
    return dll;
}

void add_dll(struct dll *dll, void *data)
{
    printf("adding to dll\n");
    struct node *node = xmalloc(sizeof(node));
    node->data = data;
    node->next = NULL;
    node->prev = dll->tail;
    dll->len++;
    printf("len is %u\n", dll->len);

    dll->tail = node;

    if (dll->len == 1U)
    {
        dll->head = node;
    }
}
void remove_dll(struct dll *dll, struct node *node)
{
    if (dll->len == 0U)
        return;

    if (node->prev != NULL)
    {
        node->prev->next = node->next;
    }
    else
    {
        dll->head = node->next;
    }
    if (node->next != NULL)
    {
        node->next->prev = node->prev;
    }
    else
    {
        dll->tail = node->prev;
    }
    free(node);
    dll->len--;
}

void print_dll(struct dll *dll)
{
    printf("------ Printing dll -------\n");
    printf("dll length is %u\n", dll->len);
    for (struct node *node = dll->head; node != NULL; node = node->next)
    {
        printf("| Prev @ %p, data @ %p, Next @ %p\n", node->prev, node->data, node->next);
    }
    printf("------ DONE: Printing dll -------\n");
}