#ifndef AABB_COMPONENT_COMPONENT_H
#define AABB_COMPONENT_COMPONENT_H

#include "component_base.h"

struct aabb_component {
  component_set set;
  struct {
    struct vec4_st *extent;
    float *radius;
  } *streams;
};

extern struct aabb_component *aabb_component;

bool initialize_aabb_component();

#endif
