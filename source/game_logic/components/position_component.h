#ifndef POSITION_COMPONENT_COMPONENT_H
#define POSITION_COMPONENT_COMPONENT_H

#include "component_base.h"

COMPONENT_DEFINE(position);
struct vec4_st *get_position(entity e) ;
struct vec4_st *get_previous_position(entity e) ;
bool set_position(entity e, float *position);
bool set_entity_position(entity e, float x, float y, float z) ;
bool initialize_position_component();
bool snapshot_positions();

#endif
