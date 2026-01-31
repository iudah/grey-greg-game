#ifndef ISTACK_H
#define ISTACK_H

#include <stdint.h>

#include "ilist.h"

typedef struct istack istack;

istack *istack_create(size_t unit_size);
void istack_destroy(istack *stack);
uint32_t istack_push(istack *stack, void *object);
void *istack_pop(istack *stack);

#endif