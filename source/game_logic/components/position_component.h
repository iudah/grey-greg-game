#ifndef POSITION_COMPONENT_COMPONENT_H
#define POSITION_COMPONENT_COMPONENT_H

#include "component_base.h"

struct position_component {
  component_set set;
  struct vec4_st *position;
  struct vec4_st *prev_position;
  struct vec4_st *curr_position;
};

extern struct position_component *position_component;

bool initialize_position_component();

#endif
