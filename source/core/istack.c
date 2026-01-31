#include "istack.h"

#include <ilist.h>
#include <stdint.h>
#include <zot.h>

struct istack {
  ilist *list;
  uint32_t top;
};

istack *istack_create(size_t unit_size) {
  istack *stack = zmalloc(sizeof(*stack));
  stack->list = ilist_create(unit_size);
  stack->top = 0;
  return stack;
}

void istack_destroy(istack *stack) {
  ilist_destroy(stack->list);
  zfree(stack);
}

uint32_t istack_push(istack *stack, void *object) {
  stack->top = ilist_append(stack->list, object);
  return stack->top - 1;
}

void *istack_pop(istack *stack) {
  if (!stack->top) return NULL;

  return ilist_get(stack->list, --stack->top);
}