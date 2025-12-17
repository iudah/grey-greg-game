#ifndef COMPONENT_H
#define COMPONENT_H

#include <stdint.h>

#include "entity.h"

typedef struct {
  entity *dense;
  uint32_t *sparse;
  uint32_t *mask;
  uint64_t *streams_sizes;
  uint32_t count;
  uint8_t no_of_stream;
} component_set;

struct generic_component {
  component_set set;
  void **streams;
};

typedef bool (*set_entity_component_value_fn)(entity e, ...);

bool set_entity_position(entity e, float x, float y, float z);
bool set_entity_velocity(entity e, float x, float y, float z);
bool set_entity_aabb_lim(entity e, float x, float y, float z);
bool set_entity_waypoint(entity e, float x, float y, float z);

bool attach_component(entity e, struct generic_component *component);
void detach_component(entity e, struct generic_component *component);
bool initialize_component(struct generic_component *component,
                          uint64_t *component_size, uint8_t no_of_stream);

static inline bool has_component(entity e,
                                 struct generic_component *component) {
  if (e.id >= MAX_NO_ENTITY)
    return false;

  uint32_t j = component->set.sparse[e.id];

  if (j >= component->set.count || component->set.dense[j].id != e.id)
    return false;

  return true;
}

#endif
