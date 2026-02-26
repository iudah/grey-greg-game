#include "velocity_component.h"

#include <stdint.h>
#include <zot.h>

struct velocity_component *velocity_component;


COMPONENT_STREAM_DEFINE(velocity, { 
  struct vec4_st *velocity; });

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

bool set_entity_velocity(entity e, float x, float y, float z) {
  return set_velocity(e, (float[]){x, y, z});
}
 struct vec4_st *get_velocity(entity e) {
  return COMPONENT_GET(velocity, e, velocity);
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
