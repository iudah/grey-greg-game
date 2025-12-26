#ifndef POSITION_COMPONENT_COMPONENT_H
#define POSITION_COMPONENT_COMPONENT_H

#include "component_base.h"

struct position_component
{
  component_set set;
  struct
  {
    struct vec4_st *position;
    struct vec4_st *prev_position;
    // `curr_position` serves for interpolation between positions
    // struct vec4_st *curr_interp_position;
    // struct vec4_st *prev_interp_position;

  } *stream;
};

extern struct position_component *position_component;

bool initialize_position_component();
bool set_position(entity e, float *position);
struct vec4_st *get_position(entity e);

static inline bool set_entity_position(entity e, float x, float y, float z)
{
  return set_position(e, (float[]){x, y, z});
}

#endif
