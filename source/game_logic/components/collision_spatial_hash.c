#include "collision_spatial_hash.h"

#include <entity.h>
#include <game_logic.h>
#include <simd.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <zot.h>

#include "collision_component.h"
#include "position_component.h"

#define MAXTILESIZE   \
  (GREY_TILE_SIZE < 2 \
       ? 2            \
       : GREY_TILE_SIZE)  // Entities larger than this will be skipped in spatial partitioning
#define SPATIAL_CELL_SIZE (GREY_TILE_SIZE)
#define GREY_TABLE_SIZE   (13297)  // Keep as prime number to reduce collisions
// is xyz the screen or world coordinates
uint32_t hash_cell_coordinates(int32_t x, int32_t y, int32_t z, uint32_t table_size) {
  uint32_t hash = (x * 92837111) ^ (y * 689287499) ^ (z * 3892886149);
  return hash % table_size;
}

struct collision_spatial_cell {
  entity *entities;
  uint32_t count;
};

struct collision_spatial_cell *cell_list;
bool __attribute__((constructor)) initialize_spatial_partition() {
  cell_list = zcalloc(GREY_TABLE_SIZE, sizeof(struct collision_spatial_cell));
  if (!cell_list) return false;
  return true;
}
bool clear_spatial_partition() {
  for (uint32_t i = 0; i < GREY_TABLE_SIZE; ++i) {
    cell_list[i].count = 0;
  }
  return true;
}

bool add_entity_to_cell(uint32_t cell_hash, entity e) {
  if (!cell_list[cell_hash].entities) {
    cell_list[cell_hash].entities = zcalloc(CELL_GROW_STEP, sizeof(entity));
  } else if ((cell_list[cell_hash].count & CELL_GROW_MASK) == 0) {
    // Resize the entities array if needed
    cell_list[cell_hash].entities =
        zrealloc(cell_list[cell_hash].entities,
                 (cell_list[cell_hash].count + CELL_GROW_STEP) * sizeof(entity));
  }
  if (!cell_list[cell_hash].entities) return false;
  cell_list[cell_hash].entities[cell_list[cell_hash].count++] = e;
  return true;
}
bool remove_entity_from_cell(uint32_t cell_hash, entity e) {
  if (!cell_list[cell_hash].entities) {
    return false;
  }
  if (!cell_list[cell_hash].count) {
    return false;
  }

  for (uint32_t i = 0; i < cell_list[cell_hash].count; ++i) {
    if (is_same_entity(cell_list[cell_hash].entities[i], e)) {
      memmove(&cell_list[cell_hash].entities[i], &cell_list[cell_hash].entities[i + 1],
              (cell_list[cell_hash].count - i - 1) * sizeof(entity));
      cell_list[cell_hash].count--;
      return true;
    }
  }
  return false;
}

struct cell_coord {
  int32_t x, y, z, unused;
};

bool compute_min_max_cell(entity e, struct vec4_st *position, struct vec4_st *extent,
                          struct cell_coord *min_cell, struct cell_coord *max_cell) {
  // Register entity to spatial partition grid based on position and extent
  float32x4_t pos = vld1q_f32((float *)position);
  float32x4_t ext = vld1q_f32((float *)extent);
  float32x4_t min = vsubq_f32(pos, ext);
  float32x4_t max = vaddq_f32(pos, ext);
  float32x4_t inv_cell_size = vdupq_n_f32(1.0f / SPATIAL_CELL_SIZE);

  max = vmulq_f32(max, inv_cell_size);
  min = vmulq_f32(min, inv_cell_size);

  float32x4_t span = vsubq_f32(max, min);
  bool skip_reg = vgetq_lane_f32(span, 0) > (float)MAXTILESIZE ||
                  vgetq_lane_f32(span, 1) > (float)MAXTILESIZE ||
                  vgetq_lane_f32(span, 2) > (float)MAXTILESIZE;
  if (skip_reg) return false;  // Entity is too large, skip registration

  int32x4_t min_s = vcvtq_s32_f32(min);
  int32x4_t max_s = vcvtq_s32_f32(max);

  // Floor correction: if truncation pushed value toward zero instead of -inf, subtract 1
  uint32x4_t min_neg = vcgtq_f32(vcvtq_f32_s32(min_s), min);
  uint32x4_t max_neg = vcgtq_f32(vcvtq_f32_s32(max_s), max);
  min_s = vsubq_s32(min_s, vandq_s32(vreinterpretq_s32_u32(min_neg), vdupq_n_s32(1)));
  max_s = vsubq_s32(max_s, vandq_s32(vreinterpretq_s32_u32(max_neg), vdupq_n_s32(1)));

  vst1q_s32((int32_t *)min_cell, min_s);
  vst1q_s32((int32_t *)max_cell, max_s);

  if (grey_is_2d()) {
    min_cell->z = 0;
    max_cell->z = 0;
  }

  return true;
}

bool register_to_spatial_partition(entity e) {
  struct vec4_st *position = get_position(e);
  struct vec4_st *extent = get_collision_extent(e);

  if (!position || !extent) return false;

  struct cell_coord min_cell = {0};
  struct cell_coord max_cell = {0};
  if (!compute_min_max_cell(e, position, extent, &min_cell, &max_cell)) {
    return false;
  }
  for (int32_t x = min_cell.x; x <= max_cell.x; ++x) {
    for (int32_t y = min_cell.y; y <= max_cell.y; ++y) {
      for (int32_t z = min_cell.z; z <= max_cell.z; ++z) {
        uint32_t cell_hash = hash_cell_coordinates(x, y, z, GREY_TABLE_SIZE);
        add_entity_to_cell(cell_hash, e);
      }
    }
  }

  return true;
}

bool deregister_from_spatial_partition_if_registered(entity e) {
  struct vec4_st *position = get_previous_position(e);
  struct vec4_st *extent = get_collision_extent(e);

  if (!position || !extent) return false;

  struct cell_coord min_cell = {0};
  struct cell_coord max_cell = {0};
  if (!compute_min_max_cell(e, position, extent, &min_cell, &max_cell)) {
    return false;
  }
  for (int32_t x = min_cell.x; x <= max_cell.x; ++x) {
    for (int32_t y = min_cell.y; y <= max_cell.y; ++y) {
      for (int32_t z = min_cell.z; z <= max_cell.z; ++z) {
        uint32_t cell_hash = hash_cell_coordinates(x, y, z, GREY_TABLE_SIZE);
        remove_entity_from_cell(cell_hash, e);
      }
    }
  }

  return true;
}

bool update_spatial_partition() {
  for (uint32_t i = 0; i < collision_component->set.count; ++i) {
    entity e = get_entity(collision_component, i);
    bool *dirty = get_collision_spatial_dirty(e);

    if (!dirty || !*dirty) continue;

    deregister_from_spatial_partition_if_registered(e);
    register_to_spatial_partition(e);
    *dirty = false;
  }
  return true;
}
