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

extern struct position_component *position_component;
extern struct velocity_component *velocity_component;
extern struct aabb_component *aabb_component;
extern struct waypoint_component *waypoint_component;

bool attach_component(entity e, struct generic_component *component);
void detach_component(entity e, struct generic_component *component);
bool initialize_component(struct generic_component *component,
                          uint64_t component_size);

#endif
