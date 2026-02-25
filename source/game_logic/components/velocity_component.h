#ifndef VELOCITY_COMPONENTS_H
#define VELOCITY_COMPONENTS_H

#include "component_base.h"

COMPONENT_DEFINE(velocity, { struct vec4_st *velocity; });

static inline bool set_entity_velocity(entity e, float x, float y, float z) {
  return set_velocity(e, (float[]){x, y, z});
}
static inline struct vec4_st *get_velocity(entity e) {
  return COMPONENT_GET(velocity_component, e, velocity);
}
bool set_velocity(entity e, float *vel);

#endif