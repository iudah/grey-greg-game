#ifndef COLLISION_COMPONENT_H
#define COLLISION_COMPONENT_H

#include <grey_constants.h>

#include "component_base.h"

COMPONENT_DEFINE(collision);

bool initialize_collision_component();
struct vec4_st *get_collision_extent(entity e);
float *get_collision_radius(entity e);
uint32_t *get_collision_layer(entity e);
uint32_t *get_collision_mask(entity e);
bool *get_collision_spatial_dirty(entity e);
bool set_entity_collision_extent(entity e, float x, float y, float z);
bool set_entity_collision_layer(entity e, uint32_t layer);
bool set_entity_collision_mask(entity e, uint32_t mask);
bool set_collision_spatial_dirty(entity e, bool dirty);
bool perform_collision_sweep_and_prune();
bool should_test_collision(entity e1, entity e2);
const entity *get_collision_sorted_entity();
const float *get_collision_sorted_min_x();
const collision_flag *get_collision_flag(entity e);
bool set_entity_collision_flag(entity e, uint32_t flag);
#endif
