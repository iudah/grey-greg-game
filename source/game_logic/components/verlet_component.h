#ifndef VERLET_COMPONENTS_H
#define VERLET_COMPONENTS_H

#include "component.h"

struct verlet_component {
  component_set set;
  struct {
    struct vec4_st {
      float x, y, z, w;
    } *acceleration;
  } *streams;
};

extern struct verlet_component *verlet_component;

bool initialize_verlet_component();

#endif