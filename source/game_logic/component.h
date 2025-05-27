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

// extern struct generic_component *position_component;
// extern struct generic_component *velocity_component;
// extern struct generic_component *rotation_component;
// extern struct generic_component *network_component;
// extern struct generic_component *ai_component;
// extern struct generic_component *physics_component;

bool attach_component(entity e, struct generic_component *component);
void detach_component(entity e, struct generic_component *component);
bool initialize_component(struct generic_component *component,
                          uint64_t component_size);

#endif
