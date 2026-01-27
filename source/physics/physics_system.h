#ifndef PHYSICS_SYSTEM_H
#define PHYSICS_SYSTEM_H

#include "entity.h"
#include "event_system.h"
#include "game_logic.h"

void physics_system_update(game_logic *logic, float delta_time);
struct vec4_st *get_position(entity e);
struct vec4_st *get_velocity(entity e);
bool set_velocity(entity e, float *vel);
void event_enqueue_collision(game_logic *logic, entity entity_i, entity entity_j);
struct vec4_st *get_next_patrol_point(entity e);
bool advance_patrol_index(entity e);
bool walk_through_resolution(event *e);

#endif