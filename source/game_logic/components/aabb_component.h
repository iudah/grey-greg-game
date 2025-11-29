#ifndef AABB_COMPONENT_COMPONENT_H
#define AABB_COMPONENT_COMPONENT_H

#include "component_base.h"

struct aabb_component {
  component_set set;
  struct vec4_st *extent;
  float *radius;
  struct vec4_st *prev_timestep_pos;
};

extern struct aabb_component *aabb_component;

bool initialize_aabb_component();

#endif
