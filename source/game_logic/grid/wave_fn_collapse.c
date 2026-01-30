#include "wave_fn_collapse.h"

#include <stdint.h>
#include <zot.h>

#include "grid.h"

struct wfc_tile {
  uint8_t socket[8];
  uint8_t tile_id;
  uint8_t solid;
  uint8_t weight;
};

struct wfc_rules_set {
  wfc_tile *tiles;
  uint32_t count;
  uint32_t net_weight;
};

static wfc_direction opposite[] = {[NORTH] = SOUTH, [EAST] = WEST, [SOUTH] = NORTH, [WEST] = EAST};

wfc_rules_set *wfc_rules_create(uint32_t no_of_rules) {
  wfc_rules_set *rules = zmalloc(sizeof(*rules));
  rules->tiles = zmalloc(no_of_rules * sizeof(*rules->tiles));
  rules->count = 0;
  rules->net_weight = 0;
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
  // tile->solid = solid;
  tile->weight = weight;

  rule->net_weight += weight;
}

bool wfc_tile_can_connect(wfc_tile *a, wfc_tile *b, wfc_direction direction) {
  return a->socket[direction] == b->socket[opposite[direction]];
}

wfc_tile *wfc_pick_tile(wfc_rules_set *wave_collapse_rule) {
  return &wave_collapse_rule->tiles[rand() % wave_collapse_rule->count];
}

void wave_fn_collapse(grid *grid, uint64_t x0, uint64_t y0, wfc_rules_set *wave_collapse_rule) {
  uint64_t size[2];
  grid_get_grid_size(grid, size);
  for (uint32_t i = 0; i < size[0]; ++i) {
    uint32_t x = x0 + i % size[0];

    for (uint32_t j = 0; j < size[1]; ++j) {
      uint32_t y = y0 + j % size[1];

      wfc_tile *a = wfc_pick_tile(wave_collapse_rule);
      // ...
    }
  }
}