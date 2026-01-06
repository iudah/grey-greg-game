#ifndef FORCE_COMPONENT_H
#define FORCE_COMPONENT_H

#include "component_base.h"

struct force_component {
  component_set set;
  struct {
    struct vec4_st* force;
  }* stream;
};

extern struct force_component* force_component;

bool initialize_force_component();
struct vec4_st* get_force(entity e);
bool apply_force(entity e, float x, float y, float z);
void clear_forces();

static inline bool add_force(entity e, float* force) {
  return apply_force(e, force[0], force[1], force[2]);
}
#endif