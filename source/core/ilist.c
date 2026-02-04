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
  if (unit_size == 0) return NULL;

  ilist *list = zmalloc(sizeof(*list));
  if (!list) return NULL;

  list->capacity = 8;
  list->count = 0;
  list->unit_size = unit_size;
  list->ptr = zmalloc(unit_size * list->capacity);

  if (!list->ptr) {
    zfree(list);
    return NULL;
  }

  return list;
}

void ilist_destroy(ilist *list) {
  zfree(list->ptr);
  zfree(list);
}

bool ilist_append(ilist *list, const void *object) {
  if (!list || !object) return false;

  if (list->count == list->capacity) {
    //  if (list->capacity > UINT32_MAX / 2) return false;
    uint32_t new_capacity = list->capacity * 2;
    void *tmp = zrealloc(list->ptr, new_capacity * list->unit_size);
    if (!tmp) return false;

    list->ptr = tmp;
    list->capacity = new_capacity;
  }

  uint8_t *dest = (uint8_t *)list->ptr + (list->count * list->unit_size);
  memcpy(dest, object, list->unit_size);
  list->count++;

  return true;
}

void *ilist_get(ilist *list, uint32_t index) {
  if (index >= list->count) return NULL;
  return (void *)(((uint8_t *)list->ptr) + index * list->unit_size);
}

bool ilist_pop(ilist *list, void *item) {
  if (list->count == 0) return false;

  --list->count;
  if (item) {
    void *src = (uint8_t *)list->ptr + (list->count * list->unit_size);
    memcpy(item, src, list->unit_size);
  }
  return true;
}

uint32_t ilist_count(ilist *list) { return list->count; }
