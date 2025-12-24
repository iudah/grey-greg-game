#ifndef VELOCITY_COMPONENTS_H
#define VELOCITY_COMPONENTS_H

#include "component_base.h"

struct velocity_component
{
  component_set set;
  struct
  {
    struct vec4_st *velocity, *acceleration;
  } *streams;
};

extern struct velocity_component *velocity_component;

bool initialize_velocity_component();

#endif