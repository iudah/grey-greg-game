#ifndef ILIST_H
#define ILIST_H

#include <stdbool.h>
#include <stdint.h>

typedef struct list ilist;

ilist *ilist_create(size_t unit_size);
uint32_t ilist_append(ilist *list, void *object);
void *ilist_get(ilist *list, uint32_t index);

#endif
