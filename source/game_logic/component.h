#ifndef COMPONENT_H
#define COMPONENT_H

#include <stdint.h>

#include "entity.h"

typedef struct {
  entity *dense;
  uint32_t *sparse;
  uint32_t *mask;
  uint32_t count;
  uint64_t component_size;
} component_set;

struct generic_component {
  component_set set;
  void *component_data;
};

typedef bool (*set_entity_component_value_fn)(entity e, ...);

extern struct position_component *position_component;
extern struct velocity_component *velocity_component;
extern struct aabb_component *aabb_component;
extern struct waypoint_component *waypoint_component;

bool set_entity_position(entity e, float x, float y, float z);
bool set_entity_velocity(entity e, float x, float y, float z);
bool set_entity_aabb_lim(entity e, float x, float y, float z);
bool set_entity_waypoint(entity e, float x, float y, float z);

bool attach_component(entity e, struct generic_component *component);
void detach_component(entity e, struct generic_component *component);
bool initialize_component(struct generic_component *component,
                          uint64_t component_size);

static inline bool has_component(entity e, struct generic_component *component) {
  if (e.id >= MAX_NO_ENTITY)
    return false;

  uint32_t j = component->set.sparse[e.id];

  if (j >= component->set.count || component->set.dense[j].id != e.id)
    return false;

  return true;
}

#endif
