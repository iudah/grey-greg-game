#include "aabb_component.h"
#include <zot.h>

struct aabb_component *aabb_component;

bool initialize_aabb_component() {
  aabb_component = zcalloc(1, sizeof(struct aabb_component));
  aabb_component->extent =
      zcalloc(MAX_NO_ENTITY, sizeof(*aabb_component->extent));
  aabb_component->radius =
      zcalloc(MAX_NO_ENTITY, sizeof(*aabb_component->radius));
  return aabb_component != NULL &&
         initialize_component((struct generic_component *)aabb_component,
                              sizeof(struct vec4_st));
}