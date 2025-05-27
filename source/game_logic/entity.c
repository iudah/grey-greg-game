#include "entity.h"

#include <stdint.h>
#include <zot.h>

#define MASK_SIZE ((MAX_NO_ENTITY + 31) / 32)

typedef struct {
  uint8_t generation;
  bool active;
} entity_record;

entity_record *entity_registry = NULL;

uint32_t *free_entities = NULL;
uint32_t number_of_free_entities = 0;

uint32_t number_of_active_entities = 0;

entity player;

entity create_entity() {
  uint32_t id;

  if (number_of_free_entities > 0) {
    id = free_entities[--number_of_free_entities];
  } else {
    if (number_of_active_entities >= MAX_NO_ENTITY) {
      LOG_ERROR("Maximum number of entities reached.");
      return (entity){0};
    }
    id = number_of_active_entities++;
  }

  if (!entity_registry)
    entity_registry = zcalloc(MAX_NO_ENTITY, sizeof(*entity_registry));
  entity_registry[id].active = true;
  return (entity){id, entity_registry[id].generation};
}

void destroy_entity(entity e) {
  if (!free_entities)
    free_entities = zcalloc(MAX_NO_ENTITY, sizeof(*free_entities));
  free_entities[number_of_free_entities++] = e.id;
  entity_registry[e.id].generation++;
  entity_registry[e.id].active = false;
}
