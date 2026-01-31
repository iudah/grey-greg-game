#ifndef WAVE_FN_COLLAPSE_H
#define WAVE_FN_COLLAPSE_H
#include <stdbool.h>
#include <stdint.h>

#include "grid.h"
typedef struct wfc_tile wfc_tile;
typedef struct wfc_rules_set wfc_rules_set;
typedef enum {
  NORTH,
  EAST,
  SOUTH,
  WEST,
  NORTH_EAST,
  SOUTH_EAST,
  SOUTH_WEST,
  NORTH_WEST,
  N_DIRECTION
} wfc_direction;

wfc_rules_set *wfc_rules_create(uint32_t no_of_rules);
void wfc_rules_destroy(wfc_rules_set *rule);
void wfc_rule_add(wfc_rules_set *rule, uint32_t tile_id, wfc_direction n, wfc_direction w,
                  wfc_direction s, wfc_direction e, uint8_t weight);
void wave_fn_collapse(grid *grid, wfc_rules_set *wave_collapse_rule);

#endif