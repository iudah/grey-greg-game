#include "wave_fn_collapse.h"

#include <irand.h>
#include <istack.h>
#include <stdint.h>
#include <string.h>
#include <zot.h>

#include "grid.h"

struct wfc_tile {
  uint32_t socket[8];
  uint32_t tile_id;
  uint8_t weight;
};

struct wfc_rules_set {
  wfc_tile *tiles;
  uint32_t count;
};

struct wave_cell {
  uint64_t *options;  // bit-masked
  uint32_t n_options;
};

struct wave_grid {
  struct wave_cell *cell;
  uint64_t *collapsed;  // bit-masked
};

#define GET_BIT(mask, id)   ((mask)[(id) / 64] & (UINT64_C(1) << ((id) % 64)))
#define SET_BIT(mask, id)   ((mask)[(id) / 64] |= (UINT64_C(1) << ((id) % 64)))
#define CLEAR_BIT(mask, id) ((mask)[(id) / 64] &= ~(UINT64_C(1) << ((id) % 64)))

static wfc_direction opposite[] = {[NORTH] = SOUTH, [EAST] = WEST, [SOUTH] = NORTH, [WEST] = EAST};

wfc_rules_set *wfc_rules_create(uint32_t no_of_rules) {
  wfc_rules_set *rules = zmalloc(sizeof(*rules));
  rules->tiles = zmalloc(no_of_rules * sizeof(*rules->tiles));
  rules->count = 0;
  return rules;
}

void wfc_rules_destroy(wfc_rules_set *rule) {
  if (!rule) return;
  zfree(rule->tiles);
  zfree(rule);
}

void wfc_rule_add(wfc_rules_set *rule, uint32_t tile_id, wfc_direction n, wfc_direction w,
                  wfc_direction s, wfc_direction e, uint8_t weight) {
  wfc_tile *tile = &rule->tiles[rule->count++];
  tile->tile_id = tile_id;
  tile->socket[NORTH] = n;
  tile->socket[SOUTH] = s;
  tile->socket[EAST] = e;
  tile->socket[WEST] = w;
  tile->weight = weight;
}

bool wfc_tile_can_connect(wfc_tile *a, wfc_tile *b, wfc_direction direction) {
  return a->socket[direction] == b->socket[opposite[direction]];
}

static inline uint32_t required_uint64s(uint32_t count) { return (count + 63) / 64; }

bool wave_cell_collapsed(struct wave_grid *grid, uint32_t cell_idx) {
  return GET_BIT(grid->collapsed, cell_idx) != 0;
}

uint32_t wave_cell_option_count(struct wave_grid *grid, uint32_t cell_idx) {
  return grid->cell[cell_idx].n_options;
}

bool wave_cell_has_rule(struct wave_grid *grid, uint32_t cell_idx, uint32_t rule_idx) {
  return GET_BIT(grid->cell[cell_idx].options, rule_idx) != 0;
}

void wave_cell_set_rule(struct wave_grid *grid, uint32_t cell_idx, uint32_t rule_idx, bool active) {
  struct wave_cell *cell = &grid->cell[cell_idx];
  bool currently_active = wave_cell_has_rule(grid, cell_idx, rule_idx);

  if (active && !currently_active) {
    SET_BIT(cell->options, rule_idx);
    cell->n_options++;
  } else if (!active && currently_active) {
    CLEAR_BIT(cell->options, rule_idx);
    cell->n_options--;
  }
}

void wave_cell_collapse(struct wave_grid *grid, uint32_t cell_idx) {
  struct wave_cell *cell = &grid->cell[cell_idx];
  cell->n_options = 1;
  SET_BIT(grid->collapsed, cell_idx);
}

struct wave_grid *wave_create(uint32_t w, uint32_t h, uint32_t n_rule) {
  uint32_t grid_size = w * h;

  struct wave_grid *grid = zmalloc(sizeof(*grid));
  grid->cell = zmalloc(sizeof(*grid->cell) * grid_size);
  grid->collapsed = zcalloc(required_uint64s(grid_size), sizeof(*grid->collapsed));
  uint32_t options_mask_words = required_uint64s(n_rule);
  for (uint32_t i = 0; i < grid_size; ++i) {
    auto cell = &grid->cell[i];
    cell->n_options = n_rule;
    cell->options = zmalloc(options_mask_words * sizeof(*cell->options));
    memset(cell->options, 0xff, options_mask_words * sizeof(*cell->options));
  }

  return grid;
}

void wave_destroy(struct wave_grid *grid, uint64_t grid_size) {
  for (uint32_t i = 0; i < grid_size; ++i) {
    struct wave_cell *cell = &grid->cell[i];
    zfree(cell->options);
  }
  zfree(grid->collapsed);
  zfree(grid->cell);
  zfree(grid);
}

bool find_lowest_entropy(struct wave_grid *wave, const uint64_t w, const uint64_t h,
                         uint32_t next_cell[2]) {
  uint32_t min_entropy = 0xffffffff;
  uint32_t idx = 0;
  bool found = false;

  for (uint64_t i = 0; i < w * h; ++i) {
    if (!wave_cell_collapsed(wave, i) && wave_cell_option_count(wave, i) < min_entropy) {
      min_entropy = wave_cell_option_count(wave, i);
      idx = i;
      found = true;
    }
  }

  if (!found) return false;

  next_cell[0] = idx % w;
  next_cell[1] = idx / w;
  return true;
}

bool collapse_cell(struct wave_grid *grid, uint32_t cell_idx, wfc_rules_set *rules) {
  uint32_t total_weight = 0;

  for (uint64_t i = 0; i < rules->count; ++i) {
    if (wave_cell_has_rule(grid, cell_idx, i)) {
      total_weight += rules->tiles[i].weight;
    }
  }

  if (total_weight == 0) {
    // wfc contradiction due to lack of options
    // backtrack or restart
    // should we keep track of restarts and auto add air tile?
    return false;
  }

  uint32_t r = irand() % total_weight;
  uint32_t idx = 0;
  while (r > rules->tiles[idx].weight) {
    r -= rules->tiles[idx].weight;
    ++idx;
  }

  for (uint32_t i = 0; i < rules->count; i++) {
    wave_cell_set_rule(grid, cell_idx, i, (i == idx));
  }

  wave_cell_collapse(grid, cell_idx);
  return true;
}

void wave_propagate(struct wave_grid *wave, uint32_t w, uint32_t h, wfc_rules_set *rules,
                    uint32_t start[2], bool is_quad_map) {
  istack *stack = istack_create(sizeof(uint32_t[2]));
  istack_push(stack, start);

  uint32_t *current_coord;

  while ((current_coord = istack_pop(stack))) {
    uint64_t c_flat_idx = current_coord[0] * w + current_coord[1];
    uint8_t dy[] = {[NORTH] = 1,      [EAST] = 0,       [SOUTH] = -1,      [WEST] = 0,
                    [NORTH_EAST] = 1, [NORTH_WEST] = 1, [SOUTH_EAST] = -1, [SOUTH_WEST] = -1};
    uint8_t dx[] = {[NORTH] = 0,      [EAST] = 1,        [SOUTH] = 0,      [WEST] = -1,
                    [NORTH_EAST] = 1, [NORTH_WEST] = -1, [SOUTH_EAST] = 1, [SOUTH_WEST] = -1};
    // wfc_direction dir[] = {[NORTH] = NORTH, [EAST] = EAST, [SOUTH] = SOUTH, [WEST] = WEST};

    for (wfc_direction d = 0; d < (is_quad_map ? 4 : N_DIRECTION); ++d) {
      int64_t nx = (int64_t)current_coord[0] * dx[d];
      int64_t ny = (int64_t)current_coord[1] * dy[d];

      // wrap map
      if (nx < 0) nx += w;
      if (ny < 0) ny += h;

      nx %= w;
      ny %= h;

      uint32_t n_idx = nx * w + ny;
      bool changed = false;

      for (uint32_t n_opt = 0; n_opt < rules->count; ++n_opt) {
        if (!wave_cell_has_rule(wave, n_idx, n_opt)) continue;

        bool is_compatible_with_remaining_rule = false;
        for (uint32_t c_opt = 0; c_opt < rules->count; ++c_opt) {
          if (!wave_cell_has_rule(wave, c_flat_idx, c_opt)) continue;

          if (wfc_tile_can_connect(&rules->tiles[c_opt], &rules->tiles[n_opt], d)) {
            is_compatible_with_remaining_rule = true;
            continue;
          }
        }

        if (!is_compatible_with_remaining_rule) {
          wave_cell_set_rule(wave, n_idx, n_opt, false);
          changed = true;
        }

        if (changed) {
          if (wave_cell_option_count(wave, n_idx) > 0) {
            istack_push(stack, (uint32_t[]){(uint32_t)nx, (uint32_t)ny});
          } else {
            // wfc contradiction due to lack of options
            // backtrack or restart
          }
        }
      }
    }
  }
  istack_destroy(stack);
}

void wave_fn_collapse(grid *grid, wfc_rules_set *rules) {
  uint64_t size[2];
  grid_get_grid_size(grid, size);

  const uint64_t w = size[0];
  const uint64_t h = size[1];

  uint32_t max_iterations = w * h + 1;
  uint32_t iterations = 0;

  if (!rules || rules->count == 0) return;

  struct wave_grid *wave = wave_create(w, h, rules->count);
  uint32_t next_cell[2];

  while (find_lowest_entropy(wave, w, h, next_cell)) {
    if (!collapse_cell(wave, next_cell[1] * w + next_cell[0], rules)) {
      // wfc contradiction due to lack of options
      // backtrack or restart
      LOG("wfc contradiction at %d, %d", next_cell[0], next_cell[1]);
      break;
    }
    wave_propagate(wave, w, h, rules, next_cell, true);

    if (++iterations > max_iterations) break;
  }

  for (uint32_t y = 0; y < h; y++) {
    for (uint32_t x = 0; x < w; x++) {
      uint64_t cell_idx = y * w + x;
      grid_cell *gc = grid_get_cell(grid, x, y);

      for (uint32_t i = 0; i < rules->count; i++) {
        if (wave_cell_has_rule(wave, cell_idx, i)) {
          grid_cell_set_tile_id(gc, rules->tiles[i].tile_id);
          break;
        }
      }
    }
  }

  wave_destroy(wave, w * h);
}