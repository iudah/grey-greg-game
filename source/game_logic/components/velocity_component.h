#ifndef VELOCITY_COMPONENTS_H
#define VELOCITY_COMPONENTS_H

#include "component.h"

struct velocity_component {
  component_set set;
  struct {
    struct vec4_st {
      float x, y, z, w;
    } *velocity, *acceleration;
  } *streams;
};

extern struct velocity_component *velocity_component;

bool initialize_velocity_component();

#endif