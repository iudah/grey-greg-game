#ifndef G_ENTITY_H
#define G_ENTITY_H

#include <stdint.h>

#define GREY_ZERO     (1e-5f)
#define GREY_COLLISION_GAP (GREY_ZERO * 1e2f)

#define INITIAL_CAPACITY (8)
#define GREY_TILE_SIZE   (16)

typedef struct {
  uint32_t id : 24;
  uint32_t generation : 8;
} entity;

extern entity player;

entity create_entity();
void destroy_entity(entity e);

void attach_position_component(entity e);
void detach_position_component(entity e);

void attach_velocity_component(entity e);
void detach_velocity_component(entity e);

bool is_same_entity(entity e1, entity e2);

#endif