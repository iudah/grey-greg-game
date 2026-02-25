#ifndef AABB_COMPONENT_COMPONENT_H
#define AABB_COMPONENT_COMPONENT_H

#include "component_base.h"

COMPONENT_DEFINE(aabb, {
  struct vec4_st *collision_extent;
  float *collision_radius;
});

static inline struct vec4_st *get_collision_extent(entity e) {
  return COMPONENT_GET(aabb_component, e, collision_extent);
}
static inline float *get_collision_radius(entity e) {
  return COMPONENT_GET(aabb_component, e, collision_radius);
}

#endif
