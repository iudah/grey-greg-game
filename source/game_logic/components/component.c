#include "component.h"

#include <assert.h>
#include <pthread.h>
#include <stdint.h>
#include <string.h>
#include <zot.h>

#include "entity.h"

#define MASK_SIZE ((MAX_NO_ENTITY + 31) / 32)

#define SET_BIT(mask, id) ((mask)[(id) / 32] |= (UINT32_C(1) << ((id) % 32)))
#define CLEAR_BIT(mask, id) ((mask)[(id) / 32] &= ~(UINT32_C(1) << ((id) % 32)))

struct {
  component_set set;
} render_component_;

struct {
  component_set set;
} physics_component_;

struct {
  component_set set;
} network_component_;

bool allocate_stream(struct generic_component *component) {
  for (uint32_t i = 0; i < component->set.no_of_stream; ++i) {
    component->streams[i] =
        zcalloc(component->set.dense_capacity, component->set.streams_sizes[i]);
  }
  return true;
}

bool reallocate_stream(struct generic_component *component, uint32_t old_cap) {
  for (uint32_t i = 0; i < component->set.no_of_stream; ++i) {
    auto tmp =
        zrealloc(component->streams[i], component->set.dense_capacity *
                                            component->set.streams_sizes[i]);
    if (!tmp)
      return false;
    component->streams[i] = tmp;

    memset(tmp + old_cap * component->set.streams_sizes[i], 0,
           (component->set.dense_capacity - old_cap) *
               component->set.streams_sizes[i]);
  }
  return true;
}

bool initialize_component(struct generic_component *component,
                          uint64_t *component_size, uint8_t no_of_stream) {
  component->set.streams_sizes =
      zmalloc(no_of_stream * sizeof(*component->set.streams_sizes));

  memcpy(component->set.streams_sizes, component_size,
         no_of_stream * sizeof(*component_size));

  component->set.count = 0;

  component->set.dense_capacity = INITIAL_CAPACITY;
  component->set.sparse_capacity = INITIAL_CAPACITY;

  component->set.dense =
      zcalloc(component->set.dense_capacity, sizeof(*component->set.dense));
  component->set.sparse =
      zcalloc(component->set.sparse_capacity, sizeof(*component->set.sparse));

  component->set.no_of_stream = no_of_stream;

  component->set.mask = zcalloc((component->set.sparse_capacity + 31) / 32,
                                sizeof(*component->set.mask));

  component->streams = zmalloc(no_of_stream * sizeof(void *));

  allocate_stream(component);

  return true;
}

bool fit_capacity(void **ptr, uint32_t *cap, uint64_t unit_size,
                  uint32_t limit) {
  if (limit < *cap)
    return true;

  auto capacity = *cap;
  while (capacity <= limit) {
    capacity *= 2;
  }

  auto tmp = zrealloc(*ptr, capacity * unit_size);
  if (!tmp)
    return false;

  *ptr = tmp;
  memset((void *)((uintptr_t)tmp + *cap * unit_size), 0,
         (capacity - *cap) * unit_size);

  *cap = capacity;

  return true;
}

bool fit_sparse_capacity(struct generic_component *component, entity e) {
  auto capacity = component->set.sparse_capacity;
  bool fit = fit_capacity((void **)&component->set.sparse,
                          &component->set.sparse_capacity,
                          sizeof(*component->set.sparse), e.id);

  if (fit && capacity != component->set.sparse_capacity) {
    auto tmp = zrealloc(component->set.mask,
                        ((component->set.sparse_capacity + 31) / 32) *
                            sizeof(*component->set.mask));
    if (!tmp)
      return false;
    component->set.mask = tmp;

    memset((void *)((uintptr_t)tmp +
                    ((capacity + 31) / 32) * sizeof(*component->set.mask)),
           0,
           (((component->set.sparse_capacity + 31) / 32) -
            ((capacity + 31) / 32)) *
               sizeof(*component->set.mask));
  }

  return fit;
}

bool fit_dense_capacity(struct generic_component *component, entity e) {
  auto capacity = component->set.dense_capacity;
  bool fit = fit_capacity((void **)&component->set.dense,
                          &component->set.dense_capacity,
                          sizeof(*component->set.dense), component->set.count);

  if (capacity != component->set.dense_capacity) {
    reallocate_stream(component, capacity);
  }

  return fit;
}

bool attach_component(entity e, struct generic_component *component) {
  fit_sparse_capacity(component, e);

  if (has_component(e, component))
    return false;

  fit_dense_capacity(component, e);

  component_set *set = &component->set;
  SET_BIT(set->mask, e.id);
  set->dense[set->count] = e;
  set->sparse[e.id] = set->count;
  set->count++;
  return true;
}

void detach_component(entity e, struct generic_component *component) {
  component_set *set = &component->set;
  for (uint8_t i = 0; i < component->set.no_of_stream; ++i) {
    uint8_t *data = (uint8_t *)component->streams[i];
    if (!has_component(e, component))
      continue;

    CLEAR_BIT(set->mask, e.id);

    uint32_t removed_idx = set->sparse[e.id];
    uint32_t last_idx = --set->count;

    entity last_entity = set->dense[last_idx];
    set->dense[removed_idx] = last_entity;
    set->sparse[last_entity.id] = removed_idx;

    if (removed_idx == last_idx)
      continue;

    assert(set->dense && set->sparse && set->mask);
    assert(component->streams[i] || component->set.streams_sizes[i] == 0);
    assert(removed_idx <= set->count);
    assert(last_idx <= set->count);

    if (component->set.streams_sizes[i] > 0)
      memcpy(&data[removed_idx * component->set.streams_sizes[i]],
             &data[last_idx * component->set.streams_sizes[i]],
             component->set.streams_sizes[i]);
  }
}
