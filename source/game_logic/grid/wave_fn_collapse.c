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

struct wfc_rule {
  wfc_tile *tiles;
  uint32_t count;
};

static wfc_direction opposite[] = {[NORTH] = SOUTH, [EAST] = WEST, [SOUTH] = NORTH, [WEST] = EAST};

wfc_rule *wfc_rules_create(uint32_t no_of_rules) {
  wfc_rule *rules = zmalloc(sizeof(*rules));
  rules->tiles = zmalloc(no_of_rules * sizeof(*rules->tiles));
  rules->count = 0;
  return rules;
}

void wfc_rules_destroy(wfc_rule *rule) {
  if (!rule) return;
  zfree(rule->tiles);
  zfree(rule);
}

void wfc_rule_add(wfc_rule *rule, uint32_t tile_id, wfc_direction n, wfc_direction w,
                  wfc_direction s, wfc_direction e, uint8_t solid) {
  wfc_tile *tile = &rule->tiles[rule->count++];
  tile->tile_id = tile_id;
  tile->socket[NORTH] = n;
  tile->socket[SOUTH] = s;
  tile->socket[EAST] = e;
  tile->socket[WEST] = w;
  tile->solid = solid;
}

bool wfc_tile_can_connect(wfc_tile *a, wfc_tile *b, wfc_direction direction) {
  return a->socket[direction] == b->socket[opposite[direction]];
}

void wave_fn_collapse(grid *grid, uint64_t x0, uint64_t y0, wfc_rule *wave_collapse_rule) {}