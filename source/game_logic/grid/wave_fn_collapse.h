#ifndef WAVE_FN_COLLAPSE_H
#define WAVE_FN_COLLAPSE_H
#include <stdbool.h>
#include <stdint.h>

#include "grid.h"
typedef struct wfc_tile wfc_tile;
typedef struct wfc_rule wfc_rule;
typedef enum {
  NORTH,
  NORTH_EAST,
  EAST,
  SOUTH_EAST,
  SOUTH,
  SOUTH_WEST,
  WEST,
  NORTH_WEST
} wfc_direction;

wfc_rule *wfc_rules_create(uint32_t no_of_rules);
void wfc_rules_destroy(wfc_rule *rule);
void wfc_rule_add(wfc_rule rule, uint32_t tile_id, wfc_direction n, wfc_direction w,
                  wfc_direction s, wfc_direction e, uint8_t solid);
void wave_fn_collapse(grid *grid, uint64_t x0, uint64_t y0, wfc_rule *wave_collapse_rule);

#endif