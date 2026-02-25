#include "velocity_component.h"

#include <stdint.h>
#include <zot.h>

struct velocity_component *velocity_component;

bool initialize_velocity_component() {
  velocity_component = zcalloc(1, sizeof(struct velocity_component));

  bool component_intialized =
      initialize_component((struct generic_component *)velocity_component,
                           (uint64_t[]){sizeof(*velocity_component->streams->velocity)},
                           sizeof(*velocity_component->streams) / sizeof(void *));

  //   velocity_component->streams->velocity =
  //       zcalloc(MAX_NO_ENTITY,
  //       sizeof(*velocity_component->streams->velocity));
  //   velocity_component->streams->acceleration = zcalloc(
  //       MAX_NO_ENTITY, sizeof(*velocity_component->streams->acceleration));

  return velocity_component != NULL && component_intialized;
}

bool set_euler_velocity(entity e, float *vel) {
  struct vec4_st *velocity = get_velocity(e);

if(!velocity) return false;

  velocity->x = vel[0];
  velocity->y = vel[1];
  velocity->z = vel[2];

  return true;
}

bool set_velocity(entity e, float *vel) { return set_euler_velocity(e, vel); }
