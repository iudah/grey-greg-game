#include "aabb_component.h"

#include <stdint.h>
#include <zot.h>

struct aabb_component *aabb_component;

bool initialize_aabb_component() {
  aabb_component = zcalloc(1, sizeof(struct aabb_component));
  auto component = (struct generic_component *)aabb_component;

  bool component_intialized =
      initialize_component(component,
                           (uint64_t[]){sizeof(*aabb_component->streams->extent),
                                        sizeof(*aabb_component->streams->radius)},
                           sizeof(*aabb_component->streams) / sizeof(void *));

  return aabb_component != NULL && component_intialized;
}
