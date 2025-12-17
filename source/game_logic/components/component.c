#include "component.h"

#include <assert.h>
#include <pthread.h>
#include <stdint.h>
#include <string.h>
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
                          uint64_t *component_size, uint8_t no_of_stream) {
  component->set.streams_sizes =
      zmalloc(no_of_stream * sizeof(*component->set.streams_sizes));
  memcpy(component->set.streams_sizes, component_size,
         no_of_stream * sizeof(*component_size));
  component->set.dense = zcalloc(MAX_NO_ENTITY, sizeof(*component->set.dense));
  component->set.sparse =
      zcalloc(MAX_NO_ENTITY, sizeof(*component->set.sparse));
  component->set.mask = zcalloc(MAX_NO_ENTITY, sizeof(*component->set.mask));

  component->streams = zmalloc(no_of_stream * sizeof(void *));

  return true;
}

bool attach_component(entity e, struct generic_component *component) {
  component_set *set = &component->set;
  if (HAS_COMPONENT(set->mask, e.id))
    return false;

  SET_BIT(set->mask, e.id);
  set->dense[set->count] = e;
  set->sparse[e.id] = set->count;
  set->count++;
  return true;
}

void detach_component(entity e, struct generic_component *component) {
  component_set *set = &component->set;
  for (uint8_t i = 0; i < component->set.no_of_stream; ++i) {
    uint8_t *data = (uint8_t *)component->streams[i];
    if (!HAS_COMPONENT(set->mask, e.id))
      continue;

    CLEAR_BIT(set->mask, e.id);

    uint32_t removed_idx = set->sparse[e.id];
    uint32_t last_idx = --set->count;

    entity last_entity = set->dense[last_idx];
    set->dense[removed_idx] = last_entity;
    set->sparse[last_entity.id] = removed_idx;

    if (removed_idx == last_idx)
      continue;

    assert(set->dense && set->sparse && set->mask);
    assert(component->streams[i] || component->set.streams_sizes[i] == 0);
    assert(removed_idx <= set->count);
    assert(last_idx <= set->count);

    if (component->set.streams_sizes[i] > 0)
      memcpy(&data[removed_idx * component->set.streams_sizes[i]],
             &data[last_idx * component->set.streams_sizes[i]],
             component->set.streams_sizes[i]);
  }
}
