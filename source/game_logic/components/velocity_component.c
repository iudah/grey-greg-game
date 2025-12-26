#include "velocity_component.h"
#include <stdint.h>
#include <zot.h>

struct velocity_component *velocity_component;

bool initialize_velocity_component()
{
  velocity_component = zcalloc(1, sizeof(struct velocity_component));

  bool component_intialized = initialize_component(
      (struct generic_component *)velocity_component,
      (uint64_t[]){sizeof(*velocity_component->streams->velocity)},
      sizeof(*velocity_component->streams) / sizeof(void *));

  //   velocity_component->streams->velocity =
  //       zcalloc(MAX_NO_ENTITY,
  //       sizeof(*velocity_component->streams->velocity));
  //   velocity_component->streams->acceleration = zcalloc(
  //       MAX_NO_ENTITY, sizeof(*velocity_component->streams->acceleration));

  return velocity_component != NULL && component_intialized;
}

bool set_euler_velocity(entity e, float *vel)
{
  if (!has_component(e, (struct generic_component *)velocity_component))

    return false;

  uint32_t j = velocity_component->set.sparse[e.id];

  velocity_component->streams->velocity[j].x = vel[0];
  velocity_component->streams->velocity[j].y = vel[1];
  velocity_component->streams->velocity[j].z = vel[2];

  return true;
}

bool set_velocity(entity e, float *vel) { return set_euler_velocity(e, vel); }

struct vec4_st *get_velocity(entity e)
{
  if (!has_component(e, (struct generic_component *)velocity_component))
    return NULL;

  uint32_t j = velocity_component->set.sparse[e.id];

  return &velocity_component->streams->velocity[j];
}