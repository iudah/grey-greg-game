#ifndef ROTATION_COMPONENT_H
#define ROTATION_COMPONENT_H

#include "component.h"

struct rotation_component {
  component_set set;
  struct {
    struct vec4_st *rotation;
  } *streams;
};

extern struct rotation_component *rotation_component;

bool initialize_rotation_component();

#endif
