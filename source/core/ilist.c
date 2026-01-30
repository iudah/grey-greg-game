#include <stdint.h>
#include <string.h>
#include <zot.h>

typedef struct list ilist;

struct list {
  void *ptr;
  size_t unit_size;
  uint32_t capacity;
  uint32_t count;
};

ilist *ilist_create(size_t unit_size) {
  ilist *list = zmalloc(sizeof(*list));
  list->capacity = 8;
  list->count = 0;
  list->unit_size = unit_size;
  list->ptr = zmalloc(unit_size * list->capacity);
  return list;
}

uint32_t ilist_append(ilist *list, void *object) {
  if (list->count == list->capacity) {
    uint32_t capacity = list->capacity * 2;
    void *tmp = zrealloc(list->ptr, capacity * list->unit_size);
    if (!tmp) {
      return 0;
    }

    list->ptr = tmp;
    list->capacity = capacity;
  }

  memcpy((void *)(((uint8_t *)list->ptr) + list->count * list->unit_size), object, list->unit_size);
  ++list->count;

  return list->count;
}

void *ilist_get(ilist *list, uint32_t index) {
  return (void *)(((uint8_t *)list->ptr) + index * list->unit_size);
}
