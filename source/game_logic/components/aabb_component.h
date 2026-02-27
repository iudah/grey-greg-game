#ifndef AABB_COMPONENT_H
#define AABB_COMPONENT_H

#include "component_base.h"

typedef enum {
  COLLISION_LAYER_PLAYER = 1 << 0,
  COLLISION_LAYER_TERRAIN = 1 << 1,
  COLLISION_LAYER_NPC = 1 << 2,
} collision_layer;

COMPONENT_DEFINE(aabb);

bool initialize_aabb_component();
struct vec4_st *get_collision_extent(entity e);
float *get_collision_radius(entity e);
uint32_t *get_collision_layer(entity e);
uint32_t *get_collision_mask(entity e);
bool set_entity_aabb_lim(entity e, float x, float y, float z);
bool set_entity_collision_layer(entity e, uint32_t layer);
bool set_entity_collision_mask(entity e, uint32_t mask);
bool belong_to_same_collision_layer(entity e1, entity e2) ;
#endif
