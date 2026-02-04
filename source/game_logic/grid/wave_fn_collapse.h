#ifndef WAVE_FN_COLLAPSE_H
#define WAVE_FN_COLLAPSE_H
#include <stdbool.h>
#include <stdint.h>

#include "grid.h"
typedef struct wfc_tile wfc_tile;
typedef struct wfc_atlas wfc_atlas;
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

wfc_atlas *wfc_atlas_create(uint16_t n_tiles);
wfc_atlas *wfc_atlas_from_sample(uint16_t n_tiles, uint32_t *sample_world, uint32_t empty_tile,
                                 uint32_t width, uint32_t height);
void wfc_atlas_destroy(wfc_atlas *rule);
void wfc_atlas_set_tile_weight(wfc_atlas *atlas, uint32_t tile_id, uint8_t weight);
void wfc_atlas_add_rule(wfc_atlas *rule, uint32_t tile_id, uint32_t n, uint32_t w, uint32_t s,
                        uint32_t e);
bool wfc_run_collapse(grid *grid, wfc_atlas *wave_collapse_rule);
void wfc_atlas_compile(wfc_atlas *atlas);

#endif