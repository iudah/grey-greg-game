#ifndef POSITION_COMPONENT_COMPONENT_H
#define POSITION_COMPONENT_COMPONENT_H

#include "component_base.h"

COMPONENT_DEFINE(position, {
  struct vec4_st *position;
  struct vec4_st *previous_position;
  // `curr_position` serves for interpolation between positions
  // struct vec4_st *curr_interp_position;
  // struct vec4_st *prev_interp_position;
});

static inline bool set_entity_position(entity e, float x, float y, float z) {
  return set_position(e, (float[]){x, y, z});
}
static inline struct vec4_st *get_position(entity e) {
  return COMPONENT_GET(position_component, e, position);
}
static inline struct vec4_st *get_previous_position(entity e) {
  return COMPONENT_GET(position_component, e, previous_position);
}

bool initialize_position_component();
bool set_position(entity e, float *position);
bool snapshot_positions();

#endif
