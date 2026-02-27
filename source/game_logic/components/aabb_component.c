#include "aabb_component.h"

#include <entity.h>
#include <physics_system.h>
#include <position_component.h>
#include <simd.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <zot.h>

struct aabb_component *aabb_component;
COMPONENT_STREAM_DEFINE(aabb, {
  struct vec4_st *collision_extent;
  float *collision_radius;
  uint32_t *collision_layer;
  uint32_t *collision_mask;
  entity *sorted_entities;
});

bool initialize_aabb_component() {
  aabb_component = zcalloc(1, sizeof(struct aabb_component));
  auto component = (struct generic_component *)aabb_component;

  bool component_intialized =
      initialize_component(component,
                           (uint64_t[]){sizeof(*aabb_component->streams->collision_extent),
                                        sizeof(*aabb_component->streams->collision_radius),
                                        sizeof(*aabb_component->streams->collision_layer),
                                        sizeof(*aabb_component->streams->collision_mask),
                                        sizeof(*aabb_component->streams->sorted_entities)},
                           sizeof(*aabb_component->streams) / sizeof(void *));

  return aabb_component != NULL && component_intialized;
}

struct vec4_st *get_collision_extent(entity e) { return COMPONENT_GET(aabb, e, collision_extent); }
float *get_collision_radius(entity e) { return COMPONENT_GET(aabb, e, collision_radius); }
uint32_t *get_collision_layer(entity e) { return COMPONENT_GET(aabb, e, collision_layer); }
uint32_t *get_collision_mask(entity e) { return COMPONENT_GET(aabb, e, collision_mask); }
bool set_entity_collision_layer(entity e, uint32_t layer) {
  uint32_t *layer_ptr = COMPONENT_GET(aabb, e, collision_layer);
  if (!layer_ptr) return false;

  *layer_ptr = layer;
  return true;
}
bool set_entity_collision_mask(entity e, uint32_t mask) {
  uint32_t *mask_ptr = COMPONENT_GET(aabb, e, collision_mask);
  if (!mask_ptr) return false;

  *mask_ptr = mask;
  return true;
}

bool belong_to_same_collision_layer(entity e1, entity e2) {
  uint32_t *layer1 = get_collision_layer(e1);
  uint32_t *layer2 = get_collision_layer(e2);
  uint32_t *mask1 = get_collision_mask(e1);
  uint32_t *mask2 = get_collision_mask(e2);

  if (!layer1 || !layer2 || !mask1 || !mask2) return false;

  return (*layer1 & *mask2) != 0 && (*layer2 & *mask1) != 0;
}

bool perform_aabb_sweep_and_prune() {
  memset(aabb_component->streams->sorted_entities, 0, aabb_component->set.count * sizeof(entity));

  float min_xs[aabb_component->set.count];
  uint32_t sorted_count = 0;

  for (uint32_t i = 0; i < aabb_component->set.count; ++i) {
    entity e = get_entity(aabb_component, i);
    struct vec4_st *position = get_position(e);
    struct vec4_st *prev_position = get_previous_position(e);
    struct vec4_st *extent = get_collision_extent(e);

    if (!position || !extent) continue;

    float32x4_t min;
    float32x4_t max;

    compute_swept_aabb_box(position, prev_position, extent, &min, &max);

    float min_x = vgetq_lane_f32(min, 0);
    uint32_t indx;
    if (i == 0) {
      min_xs[i] = min_x;
      aabb_component->streams->sorted_entities[i] = e;
    } else {
      uint32_t top = 0;
      uint32_t bottom = sorted_count;
      uint32_t mid;
      while (top < bottom) {
        mid = top + (bottom - top) / 2;
        if (min_xs[mid] > min_x) {
          bottom = mid;
        } else {
          top = mid + 1;
        }
      }

      if (is_same_entity(aabb_component->streams->sorted_entities[top], e)) {
        memmove(&min_xs[top + 1], &min_xs[top], (sorted_count - top) * sizeof(float));
        memmove(&aabb_component->streams->sorted_entities[top + 1],
                &aabb_component->streams->sorted_entities[top],
                (sorted_count - top) * sizeof(entity));
        min_xs[top] = min_x;
        aabb_component->streams->sorted_entities[top] = e;
      }
    }

    sorted_count++;
  }
}
