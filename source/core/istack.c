#include "istack.h"

#include <ilist.h>
#include <stdint.h>
#include <zot.h>

struct istack {
  ilist *list;
};

bool ilist_pop(ilist *list, void *item);

istack *istack_create(size_t unit_size) {
  istack *stack = zmalloc(sizeof(*stack));
  if (!stack) return NULL;

  stack->list = ilist_create(unit_size);
  if (!stack->list) {
    zfree(stack);
    return NULL;
  }

  return stack;
}

void istack_destroy(istack *stack) {
  ilist_destroy(stack->list);
  zfree(stack);
}

bool istack_push(istack *stack, void *object) {
  if (!ilist_append(stack->list, object)) return false;

  return true;
}

bool istack_pop(istack *stack, void *item) { return ilist_pop(stack->list, item); }