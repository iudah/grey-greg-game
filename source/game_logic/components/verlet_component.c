#include "verlet_component.h"

#include <stdint.h>
#include <zot.h>

struct verlet_component* verlet_component;

bool initialize_verlet_component() {
  verlet_component = zcalloc(1, sizeof(struct verlet_component));

  bool component_intialized = initialize_component(
      (struct generic_component*)verlet_component,
      (uint64_t[]){sizeof(*verlet_component->streams->acceleration)},
      sizeof(*verlet_component->streams) / sizeof(void*));

  // verlet_component->streams->acceleration =
  //     zcalloc(MAX_NO_ENTITY,
  //     sizeof(*verlet_component->streams->acceleration));

  return verlet_component != NULL && component_intialized;
}
