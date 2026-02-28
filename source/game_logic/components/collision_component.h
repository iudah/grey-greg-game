#ifndef COLLISION_COMPONENT_H
#define COLLISION_COMPONENT_H

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
bool belong_to_same_collision_layer(entity e1, entity e2);
#endif
