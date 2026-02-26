#ifndef VELOCITY_COMPONENTS_H
#define VELOCITY_COMPONENTS_H

#include "component_base.h"

COMPONENT_DEFINE(velocity);

bool set_velocity(entity e, float *vel);
bool set_entity_velocity(entity e, float x, float y, float z) ;
struct vec4_st *get_velocity(entity e);

#endif