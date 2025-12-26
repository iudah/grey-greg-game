#ifndef PHYSICS_SYSTEM_H
#define PHYSICS_SYSTEM_H

#include "entity.h"

void physics_system_update();
struct vec4_st *get_position(entity e);
struct vec4_st *get_velocity(entity e);
bool set_velocity(entity e, float *vel);
struct vec4_st *get_next_patrol_point(entity e);
bool advance_patrol_index(entity e);

#endif