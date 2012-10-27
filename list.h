#ifndef __LIST_H__
#define __LIST_H__

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

struct __list_node {
    char *data;
    struct __list_node *next;
};


typedef struct {
    struct __list_node *head;
} list_t;

void list_init(list_t *);
void list_clear(list_t *);
void list_add(list_t *, char *);
int list_remove(list_t *, char *);
void list_print(list_t *);

#endif // __LIST_H__
