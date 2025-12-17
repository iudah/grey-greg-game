#ifndef SCALE_COMPONENT_H
#define SCALE_COMPONENT_H

#include "component_base.h"

struct scale_component {
  component_set set;
  struct {
    struct vec4_st *scale;
  } *streams;
};

extern struct scale_component *scale_component;

bool initialize_scale_component();

#endif