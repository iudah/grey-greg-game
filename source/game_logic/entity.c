#include <stdint.h>
#include <zot.h>

#define MAX_NO_ENTITY 16777215
#define MASK_SIZE ((MAX_NO_ENTITY + 31) / 32)

typedef struct {
  uint32_t id : 24;
  uint32_t generation : 8;
} entity;

typedef struct {
  uint8_t generation;
  bool active;
} entity_record;

entity_record entity_registry[MAX_NO_ENTITY];

typedef struct {
  uint32_t dense[MAX_NO_ENTITY];
  uint32_t sparse[MAX_NO_ENTITY];
  uint32_t count;
  uint32_t mask[MASK_SIZE];
} component_set;

#define GET_BIT(mask, id) ((mask)[(id) / 32] & (1 << ((id) % 32)))
#define SET_BIT(mask, id) ((mask)[(id) / 32] |= (1 << ((id) % 32)))
#define CLEAR_BIT(mask, id) ((mask)[(id) / 32] &= ~(1 << ((id) % 32)))
#define HAS_COMPONENT(component_mask, id) GET_BIT((component_mask), (id))

struct {
  component_set set;
  float x[MAX_NO_ENTITY];
  float y[MAX_NO_ENTITY];
  float z[MAX_NO_ENTITY];
} position_component;

struct {
  component_set set;
  float x[MAX_NO_ENTITY];
  float y[MAX_NO_ENTITY];
  float z[MAX_NO_ENTITY];
} velocity_component;

uint32_t free_entities[MAX_NO_ENTITY];
uint32_t number_of_free_entities = 0;

uint32_t number_of_active_entities = 0;

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

  entity_registry[id].active = true;
  return (entity){id, entity_registry[id].generation};
}

bool attach_component(entity e, component_set *set) {
  if (HAS_COMPONENT(set->mask, e.id))
    return false;

  SET_BIT(set->mask, e.id);
  set->dense[set->count] = e.id;
  set->sparse[e.id] = set->count;
  set->count++;
  return true;
}

void detach_component(entity e, component_set *set,
                      uint32_t number_of_component_data, uint64_t *sizes,
                      uint8_t **data) {
  if (!HAS_COMPONENT(set->mask, e.id))
    return;

  CLEAR_BIT(set->mask, e.id);

  uint32_t removed_idx = set->sparse[e.id];
  uint32_t last_idx = --set->count;

  uint32_t last_entity = set->dense[last_idx];
  set->dense[removed_idx] = last_entity;
  set->sparse[last_entity] = removed_idx;

  if (removed_idx == last_idx)
    return;

  for (uint32_t i = 0; i < number_of_component_data; i++) {
    memcpy(&data[i][removed_idx * sizes[i]], &data[i][last_idx * sizes[i]],
           sizes[i]);
  }
}

void attach_position_component(entity e) {
  attach_component(e, &position_component.set);
}
void detach_position_component(entity e) {
  detach_component(e, &position_component.set, 3,
                   (uint64_t[]){sizeof(float), sizeof(float), sizeof(float)},
                   (uint8_t *[]){(uint8_t *)position_component.x,
                                 (uint8_t *)position_component.y,
                                 (uint8_t *)position_component.z});
}

void attach_velocity_component(entity e) {
  attach_component(e, &velocity_component.set);
}
void detach_velocity_component(entity e) {
  detach_component(e, &velocity_component.set, 3,
                   (uint64_t[]){sizeof(float), sizeof(float), sizeof(float)},
                   (uint8_t *[]){(uint8_t *)velocity_component.x,
                                 (uint8_t *)velocity_component.y,
                                 (uint8_t *)velocity_component.z});
}

void destroy_entity(entity e) {
  detach_position_component(e);
  detach_velocity_component(e);
  free_entities[number_of_free_entities++] = e.id;
  entity_registry[e.id].generation++;
  entity_registry[e.id].active = false;
}
