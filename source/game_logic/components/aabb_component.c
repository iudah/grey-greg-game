#include "aabb_component.h"

#include <stdint.h>
#include <zot.h>

struct aabb_component *aabb_component;
COMPONENT_STREAM_DEFINE(aabb, {
  struct vec4_st *collision_extent;
  float *collision_radius;
});

bool initialize_aabb_component() {
  aabb_component = zcalloc(1, sizeof(struct aabb_component));
  auto component = (struct generic_component *)aabb_component;

  bool component_intialized =
      initialize_component(component,
                           (uint64_t[]){sizeof(*aabb_component->streams->collision_extent),
                                        sizeof(*aabb_component->streams->collision_radius)},
                           sizeof(*aabb_component->streams) / sizeof(void *));

  return aabb_component != NULL && component_intialized;
}

struct vec4_st *get_collision_extent(entity e) {
  return COMPONENT_GET(aabb, e, collision_extent);
}
float *get_collision_radius(entity e) {
  return COMPONENT_GET(aabb, e, collision_radius);
}
