#include "wave_fn_collapse.h"

#include <collision_component.h>
#include <actor.h>
#include <component.h>
#include <inttypes.h>
#include <irand.h>
#include <istack.h>
#include <math.h>
#include <position_component.h>
#include <raylib_glue.h>
#include <render_component.h>
#include <resource_manager.h>
#include <stdint.h>
#include <string.h>
#include <zot.h>

#include "dfs_maze.h"
#include "grid.h"

typedef uint32_t wfc_key;
typedef struct wfc_registry wfc_lookup;

struct wfc_registry {
  wfc_key *keys;
  uint32_t capacity;
  uint32_t count;
};

struct wfc_atlas {
  wfc_lookup *lookup;
  uint8_t *weight;
  uint64_t *mask_cache;
  uint16_t tile_count;
};

struct wfc_slot {
  uint64_t *possibilities;  // bit-masked
  uint32_t entropy;
};

static wfc_direction opposite[] = {[NORTH] = SOUTH, [EAST] = WEST, [SOUTH] = NORTH, [WEST] = EAST};

wfc_lookup *wfc_lookup_create() {
  wfc_lookup *list = zmalloc(sizeof(*list));
  if (!list) return NULL;

  list->capacity = 8;
  list->count = 0;
  list->keys = zmalloc(sizeof(*list->keys) * list->capacity);

  if (!list->keys) {
    zfree(list);
    return NULL;
  }
  return list;
}

void wfc_lookup_destroy(wfc_lookup *list) {
  zfree(list->keys);
  zfree(list);
}

bool wfc_lookup_insert(wfc_lookup *list, const wfc_key rule_id, bool keep_unique) {
  uint32_t low = 0;
  uint32_t high = list->count;

  while (low < high) {
    uint32_t mid = low + (high - low) / 2;
    if (list->keys[mid] < rule_id) {
      low = mid + 1;
    } else {
      high = mid;
    }
  }

  if (keep_unique && low < list->count && list->keys[low] == rule_id) {
    return true;
  }

  if (list->count == list->capacity) {
    uint32_t capacity = list->capacity * 2;
    void *tmp = zrealloc(list->keys, capacity * sizeof(*list->keys));
    if (!tmp) return false;

    list->keys = tmp;
    list->capacity = capacity;
  }

  if (low < list->count) {
    memmove(&list->keys[low + 1], &list->keys[low], (list->count - low) * sizeof(*list->keys));
  }

  list->keys[low] = rule_id;
  ++list->count;

  return true;
}

bool wfc_lookup_get(wfc_lookup *list, wfc_key *rule, uint32_t index) {
  if (index >= list->count) return false;
  *rule = list->keys[index];
  return true;
}

uint32_t wfc_lookup_count(wfc_lookup *list) { return list->count; }

wfc_atlas *wfc_atlas_create(uint16_t n_tiles) {
  wfc_atlas *rules = zmalloc(sizeof(*rules));
  rules->mask_cache = NULL;
  rules->lookup = wfc_lookup_create();
  rules->weight = zmalloc(sizeof(*rules->weight) * n_tiles);
  rules->tile_count = n_tiles;
  for (uint32_t i = 0; i < n_tiles; i++) {
    rules->weight[i] = 1;
  }
  return rules;
}

wfc_atlas *wfc_atlas_from_sample(uint16_t n_tiles, uint32_t *sample_world, uint32_t empty_tile,
                                 uint32_t width, uint32_t height) {
  LOG("%s", __FUNCTION__);
  wfc_atlas *rules = wfc_atlas_create(n_tiles);

  rules->lookup = wfc_lookup_create();

  for (uint32_t y = 0; y < height; y++) {
    for (uint32_t x = 0; x < width; x++) {
      uint32_t tile = sample_world[y * width + x];
      rules->weight[tile] += 1;
    }
  }

  // prepare soft fallback for wfc
  wfc_atlas_add_rule(rules, empty_tile, empty_tile, empty_tile, empty_tile, empty_tile);

  for (uint32_t x = 0; x < width; ++x) {
    for (uint32_t y = 0; y < height; ++y) {
      uint32_t tile = sample_world[y * width + x];

      uint32_t north = (y > 0) ? sample_world[(y - 1) * width + x] : empty_tile;
      uint32_t south = (y < height - 1) ? sample_world[(y + 1) * width + x] : empty_tile;
      uint32_t west = (x > 0) ? sample_world[y * width + (x - 1)] : empty_tile;
      uint32_t east = (x < width - 1) ? sample_world[y * width + (x + 1)] : empty_tile;

      wfc_atlas_add_rule(rules, tile, north, west, south, east);
    }
  }
  return rules;
}

void wfc_atlas_destroy(wfc_atlas *rule) {
  if (!rule) return;
  wfc_lookup_destroy(rule->lookup);
  if (rule->mask_cache) {
    zfree(rule->mask_cache);
  }
  zfree(rule->weight);
  zfree(rule);
}

#define DIRECTION_MASK 0x7
#define NEIGHBOR_MASK  0x3FFF

void wfc_atlas_add_rule(wfc_atlas *rule, uint32_t tile_id, uint32_t n, uint32_t w, uint32_t s,
                        uint32_t e) {
  wfc_key rule_hash;

  uint32_t neighbors[] = {[NORTH] = n, [SOUTH] = s, [EAST] = e, [WEST] = w};

  for (wfc_direction i = 0; i < 4; i++) {
    rule_hash = (tile_id << 17) | ((i & DIRECTION_MASK) << 14) | (neighbors[i] & NEIGHBOR_MASK);
    wfc_lookup_insert(rule->lookup, rule_hash, true);

    rule_hash =
        (neighbors[i] << 17) | ((opposite[i] & DIRECTION_MASK) << 14) | (tile_id & NEIGHBOR_MASK);
    wfc_lookup_insert(rule->lookup, rule_hash, true);
  }
}

void wfc_atlas_get_allowed_mask(const wfc_atlas *atlas, uint32_t tile_id, uint32_t direction,
                                uint64_t *out_mask) {
  uint32_t words = (atlas->tile_count + 63) / 64;
  memset(out_mask, 0, words * sizeof(uint64_t));

  wfc_key search_prefix = (tile_id << 17) | ((direction & 0x7) << 14);

  uint32_t low = 0, high = atlas->lookup->count;
  while (low < high) {
    uint32_t mid = low + (high - low) / 2;
    if (atlas->lookup->keys[mid] < search_prefix)
      low = mid + 1;
    else
      high = mid;
  }

  for (uint32_t i = low; i < atlas->lookup->count; i++) {
    wfc_key current = atlas->lookup->keys[i];
    if ((current >> 14) != (search_prefix >> 14)) break;

    uint32_t neighbor_id = current & 0x3FFF;
    out_mask[neighbor_id / 64] |= (1ULL << (neighbor_id % 64));
  }
}

void wfc_atlas_set_tile_weight(wfc_atlas *atlas, uint32_t tile_id, uint8_t weight) {
  if (tile_id < atlas->tile_count) {
    atlas->weight[tile_id] = weight;
  }
}

void wfc_atlas_compile(wfc_atlas *atlas) {
  uint32_t words = required_uint64s(atlas->tile_count);
  atlas->mask_cache = zcalloc(atlas->tile_count * 4 * words, sizeof(uint64_t));

  for (uint32_t t = 0; t < atlas->tile_count; t++) {
    for (wfc_direction d = 0; d < 4; d++) {
      uint64_t *mask = &atlas->mask_cache[(t * 4 + d) * words];
      // Extract rules from your existing registry into this bitmask
      wfc_atlas_get_allowed_mask(atlas, t, d, mask);
    }
  }
}

bool wfc_slot_collapsed(struct wfc_state *grid, uint32_t cell_idx) {
  return grid->cell[cell_idx].entropy == 1;
}

uint32_t wfc_slot_count_possibilities(struct wfc_state *grid, uint32_t cell_idx) {
  return grid->cell[cell_idx].entropy;
}

bool wfc_slot_has_tile(struct wfc_state *grid, uint32_t cell_idx, uint32_t rule_idx) {
  return GET_BIT(grid->cell[cell_idx].possibilities, rule_idx) != 0;
}

void wfc_slot_set_rule(struct wfc_state *grid, uint32_t cell_idx, uint32_t rule_idx, bool active) {
  struct wfc_slot *cell = &grid->cell[cell_idx];
  bool currently_active = wfc_slot_has_tile(grid, cell_idx, rule_idx);

  if (active && !currently_active) {
    SET_BIT(cell->possibilities, rule_idx);
    cell->entropy++;
  } else if (!active && currently_active) {
    CLEAR_BIT(cell->possibilities, rule_idx);
    cell->entropy--;
  }
}

void make_cell_solid(struct wfc_state *state, wfc_atlas *atlas, resource_manager *mgr,
                     uint32_t width, uint32_t height, struct grid_coord origin) {
  for (uint32_t idx = 0; idx < atlas->tile_count; ++idx) {
    wfc_slot_set_rule(state, origin.y * width + origin.x, idx,
                      (resource_get_tile_flag(mgr, idx) & TILE_SOLID) != 0);
  }
}

void make_cell_empty(struct wfc_state *state, wfc_atlas *atlas, resource_manager *mgr,
                     uint32_t width, uint32_t height, struct grid_coord origin) {
  for (uint32_t idx = 0; idx < atlas->tile_count; ++idx) {
    wfc_slot_set_rule(state, origin.y * width + origin.x, idx,
                      (resource_get_tile_flag(mgr, idx) & TILE_WALKABLE) != 0);
  }
}

void wfc_slot_collapse(struct wfc_state *grid, uint32_t cell_idx, uint32_t chosen,
                       uint32_t tile_count) {
  struct wfc_slot *cell = &grid->cell[cell_idx];
  uint32_t words = required_uint64s(tile_count);

  memset(cell->possibilities, 0, words * sizeof(uint64_t));
  SET_BIT(cell->possibilities, chosen);
  cell->entropy = 1;
}

struct wfc_state *wfc_state_create(uint32_t w, uint32_t h, uint32_t n_tile, wfc_atlas *atlas) {
  uint32_t grid_size = w * h;

  struct wfc_state *grid = zmalloc(sizeof(*grid));
  grid->cell = zmalloc(sizeof(*grid->cell) * grid_size);
  uint32_t options_mask_words = required_uint64s(n_tile);

  for (uint32_t i = 0; i < grid_size; ++i) {
    struct wfc_slot *cell = &grid->cell[i];
    cell->entropy = n_tile;
    cell->possibilities = zcalloc(options_mask_words, sizeof(*cell->possibilities));
    for (uint32_t t = 0; t < n_tile; ++t) {
      SET_BIT(cell->possibilities, t);
    }
  }

  return grid;
}

void wave_destroy(struct wfc_state *grid, uint64_t grid_size) {
  for (uint32_t i = 0; i < grid_size; ++i) {
    struct wfc_slot *cell = &grid->cell[i];
    zfree(cell->possibilities);
  }
  zfree(grid->cell);
  zfree(grid);
}

bool wfc_find_lowest_entropy(struct wfc_state *wave, const uint64_t w, const uint64_t h,
                             struct grid_coord *least_entropy_cell) {
  uint32_t min_entropy = 0xffffffff;
  bool found = false;

  for (uint32_t y = 0; y < h; ++y) {
    for (uint32_t x = 0; x < w; ++x) {
      uint32_t i = y * w + x;

      if (wfc_slot_collapsed(wave, i) || wave->cell[i].entropy == 0) continue;

      uint32_t count = wave->cell[i].entropy;
      if (count < min_entropy) {
        min_entropy = count;
        least_entropy_cell->x = x;
        least_entropy_cell->y = y;
        found = true;
      }
    }
  }

  return found;
}

bool wave_weighted_choice(struct wfc_state *grid, uint32_t cell_idx, uint32_t *chosen_tile,
                          wfc_atlas *rules) {
  *chosen_tile = 0;
  uint32_t total_weight = 0;
  for (uint32_t i = 0; i < rules->tile_count; ++i) {
    if (!wfc_slot_has_tile(grid, cell_idx, i)) continue;

    total_weight += rules->weight[i];
  }

  if (!total_weight) {
    return false;
  }

  uint32_t r = irand() % total_weight;
  uint32_t cumulative = 0;

  for (uint32_t i = 0; i < rules->tile_count; ++i) {
    if (!wfc_slot_has_tile(grid, cell_idx, i)) continue;
    cumulative += rules->weight[i];
    if (r < cumulative) {
      *chosen_tile = i;
      return true;
    }
  }

  return false;
}

bool wfc_state_collapse_slot(struct wfc_state *grid, uint32_t cell_idx, wfc_atlas *rules) {
  if (wfc_slot_collapsed(grid, cell_idx) || grid->cell[cell_idx].entropy == 0) return false;
  uint32_t selected_tile;
  if (!wave_weighted_choice(grid, cell_idx, &selected_tile, rules)) return false;

  wfc_slot_collapse(grid, cell_idx, selected_tile, rules->tile_count);

  return true;
}

bool get_neighbor(struct grid_coord curr, wfc_direction d, uint32_t w, uint32_t h,
                  struct grid_coord *neigh_coord) {
  // Directions for exploration
  static int8_t dy[] = {[NORTH] = -1,      [EAST] = 0,        [SOUTH] = 1,      [WEST] = 0,
                        [NORTH_EAST] = -1, [NORTH_WEST] = -1, [SOUTH_EAST] = 1, [SOUTH_WEST] = 1};
  static int8_t dx[] = {[NORTH] = 0,      [EAST] = 1,        [SOUTH] = 0,      [WEST] = -1,
                        [NORTH_EAST] = 1, [NORTH_WEST] = -1, [SOUTH_EAST] = 1, [SOUTH_WEST] = -1};

  int64_t nx = (int64_t)curr.x + dx[d];
  int64_t ny = (int64_t)curr.y + dy[d];

  // Bounds check
  if (nx < 0 || ny < 0 || nx >= w || ny >= h) return false;

#if 0
    // wrap map
      if (nx < 0) nx += w;
      nx %= w;

      if (ny < 0) ny += h;
      ny %= h;
#endif

  neigh_coord->x = nx;
  neigh_coord->y = ny;

  return true;
}

bool wfc_slot_intersect(struct wfc_state *state, uint32_t neigh_idx, uint32_t words,
                        uint64_t *combined_allowed, struct wfc_atlas *atlas) {
  bool changed = false;
  struct wfc_slot *cell = &state->cell[neigh_idx];
  uint64_t *old = state->cell[neigh_idx].possibilities;
  uint64_t new;
  uint32_t new_entropy = 0;

  for (uint32_t i = 0; i < words; i++) {
    new = old[i] & combined_allowed[i];

    if (new != old[i]) {
      uint64_t removed = new ^ old[i];

      while (removed) {
        uint32_t trailing_zero = __builtin_ctzll(removed);
        uint32_t tile_id = (i * 64) + trailing_zero;

        cell->entropy--;
        removed &= ~(1ULL << trailing_zero);
      }

      old[i] = new;
      changed = true;
    }
  }

  return changed;
}

uint32_t wfc_atlas_tile_count(wfc_atlas *atlas) { return atlas->tile_count; }

bool wfc_state_propagate(struct wfc_state *state, wfc_atlas *atlas, uint32_t width, uint32_t height,
                         struct grid_coord origin) {
  istack *stack = istack_create(sizeof(struct grid_coord));
  istack_push(stack, &origin);

  uint32_t words = required_uint64s(atlas->tile_count);
  uint64_t *dir_unions = zmalloc(4 * words * sizeof(uint64_t));

  struct grid_coord curr;
  bool success = true;

  while (istack_pop(stack, &curr)) {
    uint32_t curr_idx = curr.y * width + curr.x;

    memset(dir_unions, 0, 4 * words * sizeof(uint64_t));
    for (uint32_t wd = 0; wd < words; ++wd) {
      uint64_t current_tiles = state->cell[curr_idx].possibilities[wd];

      while (current_tiles) {
        //  __builtin_ctzll(0xf0==0b11110000) counts trailing zeros  = 8
        uint32_t filt_tiles = __builtin_ctzll(current_tiles);
        uint32_t tile_id = (wd * 64) + filt_tiles;

        for (wfc_direction d = 0; d < 4; ++d) {
          uint64_t *union_ptr = &dir_unions[d * words];
          uint64_t *cached_mask = &atlas->mask_cache[(tile_id * 4 + d) * words];

          for (uint32_t i = 0; i < words; ++i) {
            union_ptr[i] |= cached_mask[i];
          }
        }

        current_tiles &= ~(1ULL << filt_tiles);
      }
    }

    for (wfc_direction d = 0; d < 4; ++d) {
      struct grid_coord neigh_coord;
      if (!get_neighbor(curr, d, width, height, &neigh_coord)) continue;

      uint32_t neigh_idx = neigh_coord.y * width + neigh_coord.x;
      if (wfc_slot_collapsed(state, neigh_idx)) continue;

      if (wfc_slot_intersect(state, neigh_idx, words, &dir_unions[d * words], atlas)) {
        if (wfc_slot_count_possibilities(state, neigh_idx) == 0) {
          // contradiction
          success = false;
          break;
        }
        istack_push(stack, &neigh_coord);
      }
    }

    if (!success) break;
  }

  zfree(dir_unions);
  istack_destroy(stack);

  return success;
}

bool wfc_run_collapse(grid *grid, wfc_atlas *rules, resource_manager *mgr, uint32_t *start_pt) {
  LOG("%s", __FUNCTION__);

  uint64_t size[2];
  grid_get_grid_size(grid, size);

  const uint64_t width = size[0];
  const uint64_t height = size[1];

  if (!rules || wfc_lookup_count(rules->lookup) == 0) return false;

  bool clear_mask_cache = false;
  if (!rules->mask_cache) {
    clear_mask_cache = true;
    wfc_atlas_compile(rules);
  }

  struct wfc_state *wave = wfc_state_create(width, height, rules->tile_count, rules);
  struct grid_coord cell_to_collapse;
  if (!dfs_generate_maze(wave, width, height, start_pt[0], start_pt[1], rules, mgr)) {
    wave_destroy(wave, width * height);
    if (clear_mask_cache) {
      zfree(rules->mask_cache);
    }
    return false;
  }

  LOG("dfs_generate_maze");

#if 1
  while (wfc_find_lowest_entropy(wave, width, height, &cell_to_collapse)) {
    if (!wfc_state_collapse_slot(wave, cell_to_collapse.y * width + cell_to_collapse.x, rules) ||
        !wfc_state_propagate(wave, rules, width, height, cell_to_collapse)) {
      LOG("wfc contradiction at %d, %d", cell_to_collapse.x, cell_to_collapse.y);
      wave_destroy(wave, width * height);
      return false;
    }
  }
#endif

  for (uint32_t y = 0; y < height; y++) {
    for (uint32_t x = 0; x < width; x++) {
      uint64_t cell_idx = y * width + x;
      grid_cell *gc = grid_get_cell(grid, x, y);

#if 0

      if (wfc_slot_collapsed(wave, y * width + x)) {
        grid_cell_set_tile_id(gc, 0);
      } else {
        grid_cell_set_tile_id(gc, 1);
      }

#else
      for (uint32_t i = 0; i < rules->tile_count; i++) {
        if (wfc_slot_has_tile(wave, cell_idx, i)) {
          grid_cell_set_tile_id(gc, i);
          break;
        }
      }
#endif
    }
  }

  wave_destroy(wave, width * height);
  if (clear_mask_cache) {
    zfree(rules->mask_cache);
  }

  return true;
}
