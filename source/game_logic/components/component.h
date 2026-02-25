#ifndef COMPONENT_H
#define COMPONENT_H

#include <entity.h>
#include <stddef.h>
#include <stdint.h>

#define GET_BIT(mask, id)   ((mask)[(id) / 64] & (UINT64_C(1) << ((id) % 64)))
#define SET_BIT(mask, id)   ((mask)[(id) / 64] |= (UINT64_C(1) << ((id) % 64)))
#define CLEAR_BIT(mask, id) ((mask)[(id) / 64] &= ~(UINT64_C(1) << ((id) % 64)))

static inline uint32_t required_uint64s(uint32_t count) { return (count + 63) / 64; }

typedef struct {
  entity *dense;
  uint32_t *sparse;
  uint64_t *mask;
  uint64_t *streams_sizes;
  uint32_t count;
  uint32_t sparse_capacity;
  uint32_t dense_capacity;
  uint8_t no_of_stream;
} component_set;

struct generic_component {
  component_set set;
  void **streams;
};

typedef struct generic_component generic_component_t;

typedef bool (*set_entity_component_value_fn)(entity e, ...);

bool set_entity_aabb_lim(entity e, float x, float y, float z);
bool set_entity_waypoint(entity e, float x, float y, float z);

bool attach_component(entity e, struct generic_component *component);
void detach_component(entity e, struct generic_component *component);
bool initialize_component(struct generic_component *component, uint64_t *component_size,
                          uint8_t no_of_stream);

static inline entity get_entity(struct generic_component *component, uint32_t id) {
  return component->set.dense[id];
}

static inline bool has_component(entity e, struct generic_component *component) {
  if (e.id >= component->set.sparse_capacity) return false;

  return GET_BIT((component->set.mask), (e.id));
}

static inline bool component_get_dense_id(struct generic_component *component, entity entity,
                                          uint32_t *dense_id) {
  if (!has_component(entity, component)) return false;

  auto dense_idx = component->set.sparse[entity.id];

  if (dense_idx > component->set.count) return false;

  *dense_id = dense_idx;
  return true;
}

static inline void *component_get_stream_ptr(struct generic_component *component, entity e,
                                             uint8_t stream_idx, size_t element_size) {
  uint32_t dense_id;
  if (!component_get_dense_id(component, e, &dense_id)) return NULL;
  return (uint8_t *)component->streams[stream_idx] + dense_id * element_size;
}

#define COMPONENT_GET(component, e, stream_field)                                 \
  (__typeof__((component)->streams->stream_field + 0))component_get_stream_ptr(   \
      (struct generic_component *)(component), (e),                               \
      offsetof(__typeof__(*(component)->streams), stream_field) / sizeof(void *), \
      sizeof(*(component)->streams->stream_field))

#define COMPONENT_DEFINE(NAME, STRUCT_NAME)         \
  struct NAME##_component {                         \
    component_set set;                              \
    struct STRUCT_NAME *streams;                    \
  };                                                \
                                                    \
  extern struct NAME##_component *NAME##_component; \
  bool initialize_##NAME##_component();

#endif
