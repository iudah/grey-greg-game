#ifndef ROTATION_COMPONENT_H
#define ROTATION_COMPONENT_H

#include "component.h"

struct rotation_component {
  component_set set;
  struct {
    struct vec4_st *rotation;
  } *streams;
};

#endif
