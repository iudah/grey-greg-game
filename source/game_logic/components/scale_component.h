#ifndef SCALE_COMPONENT_H
#define SCALE_COMPONENT_H

#include "component.h"

struct scale_component {
  component_set set;
  struct vec4_st *scale;
};

extern struct scale_component *scale_component;

bool initialize_scale_component();

#endif