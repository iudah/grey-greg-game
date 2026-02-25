#ifndef FORCE_COMPONENT_H
#define FORCE_COMPONENT_H

#include "component_base.h"

COMPONENT_DEFINE(force, { struct vec4_st *force; });

static inline bool add_force(entity e, float *force) {
  return apply_force(e, force[0], force[1], force[2]);
}
static inline struct vec4_st *get_force(entity e) {
  return COMPONENT_GET(force_component, e, force);
}

bool initialize_force_component();
bool apply_force(entity e, float x, float y, float z);
void clear_forces();

#endif