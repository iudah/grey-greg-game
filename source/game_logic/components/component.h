#ifndef COMPONENT_H
#define COMPONENT_H

#include <entity.h>
#include <stdint.h>

#define GET_BIT(mask, id) ((mask)[(id) / 32] & (UINT32_C(1) << ((id) % 32)))

typedef struct {
  entity* dense;
  uint32_t* sparse;
  uint32_t* mask;
  uint64_t* streams_sizes;
  uint32_t count;
  uint32_t sparse_capacity;
  uint32_t dense_capacity;
  uint8_t no_of_stream;
} component_set;

struct generic_component {
  component_set set;
  void** streams;
};

typedef bool (*set_entity_component_value_fn)(entity e, ...);

bool set_entity_aabb_lim(entity e, float x, float y, float z);
bool set_entity_waypoint(entity e, float x, float y, float z);

bool attach_component(entity e, struct generic_component* component);
void detach_component(entity e, struct generic_component* component);
bool initialize_component(struct generic_component* component,
                          uint64_t* component_size, uint8_t no_of_stream);

static inline entity get_entity(struct generic_component* component,
                                uint32_t id) {
  return component->set.dense[id];
}

static inline bool has_component(entity e,
                                 struct generic_component* component) {
  if (e.id >= component->set.sparse_capacity) return false;

  return GET_BIT((component->set.mask), (e.id));
}

static inline bool component_get_dense_id(struct generic_component* component,
                                          entity entity, uint32_t* dense_id) {
  if (!has_component(entity, component)) return false;

  auto dense_idx = component->set.sparse[entity.id];

  if (dense_idx > component->set.count) return false;

  *dense_id = dense_idx;
  return true;
}

#endif
