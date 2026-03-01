#ifndef PHYSICS_SYSTEM_H
#define PHYSICS_SYSTEM_H

#include <simd.h>

#include "entity.h"
#include "event_system.h"
#include "game_logic.h"

void physics_system_update(game_logic *logic, float delta_time);
bool set_velocity(entity e, float *vel);
void event_enqueue_collision(game_logic *logic, entity entity_i, entity entity_j);
struct vec4_st *get_next_patrol_point(entity e);
bool advance_patrol_index(entity e);
void compute_swept_collision_box(struct vec4_st *curr_pos, struct vec4_st *prev_pos,
                                 struct vec4_st *extent, float32x4_t *out_min,
                                 float32x4_t *out_max);

#endif