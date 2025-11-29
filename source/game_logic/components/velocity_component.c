#include "velocity_component.h"
#include <zot.h>

struct velocity_component *velocity_component;

bool initialize_velocity_component() {
  velocity_component = zcalloc(1, sizeof(struct velocity_component));
  velocity_component->velocity =
      zcalloc(MAX_NO_ENTITY, sizeof(*velocity_component->velocity));
  return velocity_component != NULL &&
         initialize_component((struct generic_component *)velocity_component,
                              sizeof(struct vec4_st));
}
