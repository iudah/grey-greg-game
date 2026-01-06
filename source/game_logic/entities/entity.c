#include "entity.h"

#include <stdint.h>
#include <zot.h>

#define MASK_SIZE ((MAX_NO_ENTITY + 31) / 32)

typedef struct {
  uint8_t generation;
  bool active;
} entity_record;

entity_record* entity_registry = NULL;

uint32_t* free_entities = NULL;
uint32_t number_of_free_entities = 0;
uint32_t capacity_of_free_entities = INITIAL_CAPACITY;

uint32_t number_of_active_entities = 0;
uint32_t capacity_of_active_entities = INITIAL_CAPACITY;

entity player;

entity create_entity() {
  uint32_t id;

  if (number_of_free_entities > 0) {
    id = free_entities[--number_of_free_entities];
  } else {
    // if (number_of_active_entities >= MAX_NO_ENTITY) {
    //   LOG_ERROR("Maximum number of entities reached.");
    //   return (entity){0};
    // }
    id = number_of_active_entities++;
  }

  if (!entity_registry) {
    entity_registry =
        zcalloc(capacity_of_active_entities, sizeof(*entity_registry));
  }
  if (number_of_active_entities >= capacity_of_active_entities) {
    auto cap = capacity_of_active_entities * 2;
    auto tmp = zrealloc(entity_registry, cap * sizeof(*entity_registry));
    if (!tmp) {
      return (entity){0};
    }
    entity_registry = tmp;
    capacity_of_active_entities = cap;
  }

  entity_registry[id].active = true;
  return (entity){id, entity_registry[id].generation};
}

void destroy_entity(entity e) {
  if (!free_entities) {
    free_entities = zcalloc(capacity_of_free_entities, sizeof(*free_entities));
  }

  if (number_of_free_entities >= capacity_of_free_entities) {
    auto cap = capacity_of_free_entities * 2;
    auto tmp = zrealloc(free_entities, cap * sizeof(*free_entities));
    if (!tmp) {
      return;
    }
    free_entities = tmp;
    capacity_of_free_entities = cap;
  }

  free_entities[number_of_free_entities++] = e.id;
  entity_registry[e.id].generation++;
  entity_registry[e.id].active = false;
}
