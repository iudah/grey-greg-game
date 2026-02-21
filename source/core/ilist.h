#ifndef ILIST_H
#define ILIST_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

typedef struct list ilist;

ilist *ilist_create(size_t unit_size);
void ilist_destroy(ilist *list);
bool ilist_append(ilist *list, void *object);
void *ilist_get(ilist *list, uint32_t index);
uint32_t ilist_count(ilist *list);

#endif
