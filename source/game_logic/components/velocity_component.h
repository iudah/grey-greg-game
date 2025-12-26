#ifndef VELOCITY_COMPONENTS_H
#define VELOCITY_COMPONENTS_H

#include "component_base.h"

struct velocity_component
{
  component_set set;
  struct
  {
    struct vec4_st *velocity;
  } *streams;
};

extern struct velocity_component *velocity_component;

bool initialize_velocity_component();
bool set_velocity(entity e, float *vel);
struct vec4_st *get_velocity(entity e);

static inline bool set_entity_velocity(entity e, float x, float y, float z)
{
  return set_velocity(e, (float[]){x, y, z});
}

#endif