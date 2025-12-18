#ifndef G_ENTITY_H
#define G_ENTITY_H

#include <stdint.h>

#define INITIAL_CAPACITY (32)

typedef struct {
  uint32_t id : 24;
  uint32_t generation : 8;
} entity;

entity create_entity();
void destroy_entity(entity e);

void attach_position_component(entity e);
void detach_position_component(entity e);

void attach_velocity_component(entity e);
void detach_velocity_component(entity e);

#endif