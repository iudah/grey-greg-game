#include "component.h"

#include <assert.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <zot.h>

#include "entity.h"

#define MASK_SIZE ((MAX_NO_ENTITY + 63) / 64)

// #define SET_BIT(mask, id)   ((mask)[(id) / 32] |= (UINT32_C(1) << ((id) % 32)))
// #define CLEAR_BIT(mask, id) ((mask)[(id) / 32] &= ~(UINT32_C(1) << ((id) % 32)))

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
    component->streams[i] = zcalloc(component->set.dense_capacity, component->set.streams_sizes[i]);
  }
  return true;
}

bool reallocate_stream(struct generic_component *component, uint32_t old_cap) {
  for (uint32_t i = 0; i < component->set.no_of_stream; ++i) {
    auto tmp = zrealloc(component->streams[i],
                        component->set.dense_capacity * component->set.streams_sizes[i]);
    if (!tmp) return false;
    component->streams[i] = tmp;

    memset(tmp + old_cap * component->set.streams_sizes[i], 0,
           (component->set.dense_capacity - old_cap) * component->set.streams_sizes[i]);
  }
  return true;
}

bool initialize_component(struct generic_component *component, uint64_t *component_size,
                          uint8_t no_of_stream) {
  component->set.streams_sizes = zmalloc(no_of_stream * sizeof(*component->set.streams_sizes));

  memcpy(component->set.streams_sizes, component_size, no_of_stream * sizeof(*component_size));

  component->set.count = 0;

  component->set.dense_capacity = INITIAL_CAPACITY;
  component->set.sparse_capacity = INITIAL_CAPACITY;

  component->set.dense = zcalloc(component->set.dense_capacity, sizeof(*component->set.dense));
  component->set.sparse = zcalloc(component->set.sparse_capacity, sizeof(*component->set.sparse));

  component->set.no_of_stream = no_of_stream;

  component->set.mask =
      zcalloc(required_uint64s(component->set.sparse_capacity), sizeof(*component->set.mask));

  component->streams = zmalloc(no_of_stream * sizeof(void *));

  allocate_stream(component);

  return true;
}

bool fit_capacity(void **ptr, uint32_t *cap, uint64_t unit_size, uint32_t limit) {
  if (limit < *cap) return true;

  auto capacity = *cap;
  while (capacity <= limit) {
    capacity *= 2;
  }

  auto tmp = zrealloc(*ptr, capacity * unit_size);
  if (!tmp) return false;

  *ptr = tmp;
  memset((void *)((uintptr_t)tmp + *cap * unit_size), 0, (capacity - *cap) * unit_size);

  *cap = capacity;

  return true;
}

bool fit_sparse_capacity(struct generic_component *component, entity e) {
  auto capacity = component->set.sparse_capacity;
  bool fit = fit_capacity((void **)&component->set.sparse, &component->set.sparse_capacity,
                          sizeof(*component->set.sparse), e.id);

  if (fit && capacity != component->set.sparse_capacity) {
    auto tmp = zrealloc(component->set.mask, required_uint64s(component->set.sparse_capacity) *
                                                 sizeof(*component->set.mask));
    if (!tmp) return false;
    component->set.mask = tmp;

    void *dest =
        (void *)((uintptr_t)tmp + required_uint64s(capacity) * sizeof(*component->set.mask));
    size_t length =
        (required_uint64s(component->set.sparse_capacity) - required_uint64s(capacity)) *
        sizeof(*component->set.mask);

    memset(dest, 0, length);
  }

  return fit;
}

bool fit_dense_capacity(struct generic_component *component, entity e) {
  auto capacity = component->set.dense_capacity;
  bool fit = fit_capacity((void **)&component->set.dense, &component->set.dense_capacity,
                          sizeof(*component->set.dense), component->set.count);

  if (capacity != component->set.dense_capacity) {
    reallocate_stream(component, capacity);
  }

  return fit;
}

bool attach_component(entity e, struct generic_component *component) {
  fit_sparse_capacity(component, e);

  if (has_component(e, component)) return false;

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
    if (!has_component(e, component)) continue;

    CLEAR_BIT(set->mask, e.id);

    uint32_t removed_idx = set->sparse[e.id];
    uint32_t last_idx = --set->count;

    entity last_entity = set->dense[last_idx];
    set->dense[removed_idx] = last_entity;
    set->sparse[last_entity.id] = removed_idx;

    if (removed_idx == last_idx) continue;

    assert(set->dense && set->sparse && set->mask);
    assert(component->streams[i] || component->set.streams_sizes[i] == 0);
    assert(removed_idx <= set->count);
    assert(last_idx <= set->count);

    if (component->set.streams_sizes[i] > 0)
      memcpy(&data[removed_idx * component->set.streams_sizes[i]],
             &data[last_idx * component->set.streams_sizes[i]], component->set.streams_sizes[i]);
  }
}

#if 1
#include <zot.h>

#include "aabb_component.h"
#include "component_manager.h"
#include "force_component.h"
#include "grid_component.h"
#include "mass_component.h"
#include "position_component.h"
#include "render_component.h"
#include "rotation_component.h"
#include "scale_component.h"
#include "velocity_component.h"
#include "waypoint_component.h"
#endif

// Todo: let game choose components to init
bool initialize_all_components() {
  LOG("Initing stuff...\n");
  return initialize_position_component() && initialize_velocity_component() &&
         initialize_aabb_component() && initialize_waypoint_component() &&
         initialize_rotation_component() && initialize_scale_component() &&
         initialize_render_component() && initialize_force_component() &&
         initialize_mass_component() && initialize_grid_component();
}

void cleanup_all_components() {
  // ToDo: Add cleanup functions for each component
}

void static __attribute__((constructor(203))) __attribute__((__used__))
init_component_manager_tl() {
  initialize_all_components();
}