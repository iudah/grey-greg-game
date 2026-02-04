#ifndef ISTACK_H
#define ISTACK_H

#include <stdint.h>

#include "ilist.h"

typedef struct istack istack;

istack *istack_create(size_t unit_size);
void istack_destroy(istack *stack);
bool istack_push(istack *stack, void *object);
bool istack_pop(istack *stack, void *item);

#endif