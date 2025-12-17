#ifndef AABB_COMPONENT_COMPONENT_H
#define AABB_COMPONENT_COMPONENT_H

#include "component_base.h"

struct aabb_component {
  component_set set;
  struct {
    struct vec4_st *extent;
    float *radius;
    struct vec4_st *prev_timestep_pos;
  } *streams;
};

// struct generic_component {
//   component_set set;
//   void **streams;
//   uint64_t *streams_sizes;
//   uint8_t no_of_stream;
// };

extern struct aabb_component *aabb_component;

bool initialize_aabb_component();

#endif
