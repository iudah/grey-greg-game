#include "component.h"

#include <stdint.h>
#include <zot.h>

#include "entity.h"

#define MASK_SIZE ((MAX_NO_ENTITY + 31) / 32)

#define GET_BIT(mask, id) ((mask)[(id) / 32] & (1 << ((id) % 32)))
#define SET_BIT(mask, id) ((mask)[(id) / 32] |= (1 << ((id) % 32)))
#define CLEAR_BIT(mask, id) ((mask)[(id) / 32] &= ~(1 << ((id) % 32)))
#define HAS_COMPONENT(component_mask, id) GET_BIT((component_mask), (id))

struct {
  component_set set;
} render_component_;

struct {
  component_set set;
} physics_component_;

struct {
  component_set set;
} network_component_;

bool initialize_component(struct generic_component *component,
                          uint64_t component_size) {
  component->set.component_size = component_size;
  component->set.dense = zcalloc(MAX_NO_ENTITY, sizeof(*component->set.dense));
  component->set.sparse =
      zcalloc(MAX_NO_ENTITY, sizeof(*component->set.sparse));
  component->set.mask = zcalloc(MAX_NO_ENTITY, sizeof(*component->set.mask));

  return true;
}

bool attach_component(entity e, struct generic_component *component) {
  component_set *set = &component->set;
  if (HAS_COMPONENT(set->mask, e.id))
    return false;

  SET_BIT(set->mask, e.id);
  set->dense[set->count] = e.id;
  set->sparse[e.id] = set->count;
  set->count++;
  return true;
}

void detach_component(entity e, struct generic_component *component) {
  component_set *set = &component->set;
  uint8_t *data = (uint8_t *)component->component_data;
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

  if (set->component_size > 0)
    memcpy(&data[removed_idx * set->component_size],
           &data[last_idx * set->component_size], set->component_size);
}
