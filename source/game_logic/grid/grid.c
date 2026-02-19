#include "grid.h"

#include <aabb_component.h>
#include <actor.h>
#include <irand.h>
#include <position_component.h>
#include <raylib_glue.h>
#include <render_component.h>
#include <stdint.h>
#include <zot.h>

struct grid_cell {
  int8_t y_offset;
  uint32_t tile_text_id;
};

struct grid {
  RenderTexture2D *cache;
  struct grid_cell *grid;  // flattened grid
  uint64_t x, y;
  uint8_t cell_width;
  uint8_t cell_height;
};

grid *grid_create(uint64_t width, uint64_t height, uint64_t cell_width, uint64_t cell_height) {
  grid *g = zmalloc(sizeof(*g));
  g->grid = zcalloc(width * height, sizeof(*g->grid));

  g->cache = 0;
  g->x = width;
  g->y = height;

  g->cell_width = cell_width;
  g->cell_height = cell_height;

  return g;
}

void grid_destroy(grid *g) {
  zfree(g->grid);
  zfree(g);
}

grid_cell *grid_get_cell(grid *g, uint64_t x, uint64_t y) {
  if (x < 0 || x >= g->x || y < 0 || y >= g->y) {
    return NULL;
  }

  return &g->grid[x * g->y + y];
}

void grid_get_grid_size(grid *g, uint64_t size[2]) {
  size[0] = g->x;
  size[1] = g->y;
}

void grid_set_grid_cache(grid *g, RenderTexture2D *cache) { g->cache = cache; }

RenderTexture2D *grid_get_grid_cache(grid *g) { return g->cache; }

RenderTexture2D *grid_bake(grid *grid, resource_manager *resc_mgr) {
  RenderTexture2D *cache = zmalloc(sizeof(RenderTexture2D));
  *cache = LoadRenderTexture(grid->x * grid->cell_width, grid->y * grid->cell_width);

  BeginTextureMode(*cache);
  ClearBackground(BLANK);

  for (uint32_t x = 0; x < grid->x; ++x) {
    for (uint32_t y = 0; y < grid->y; ++y) {
      grid_cell *cell = grid_get_cell(grid, x, y);

      if (cell->tile_text_id == 0) continue;

      Vector2 position = {x * grid->cell_width, y * grid->cell_height};

      Texture2D *tex = resource_get_tile_texture(resc_mgr, cell->tile_text_id);
      if (!tex) continue;

      Rectangle *rect = resource_get_tile_rect(resc_mgr, cell->tile_text_id);
      if (!rect) continue;

      DrawTextureRec(*tex, *rect, position, WHITE);

      if ((resource_get_tile_flag(resc_mgr, cell->tile_text_id) & TILE_SOLID) != 0) {
        entity platform = create_entity();

        actor_add_component(platform, (generic_component_t *)position_component);
        actor_add_component(platform, (generic_component_t *)aabb_component);
        actor_add_component(platform, (generic_component_t *)render_component);

        uint32_t tile_w = resource_get_tile_rect(resc_mgr, cell->tile_text_id)->width;
        uint32_t tile_h = resource_get_tile_rect(resc_mgr, cell->tile_text_id)->height;

        float center_x = ((float)x * tile_w) + ((float)tile_w / 2.0f);
        float center_y = ((float)y * tile_h) + ((float)tile_h / 2.0f);

        set_entity_position(platform, center_x, center_y, 0);
        set_entity_aabb_lim(platform, (float)tile_w / 2, (float)tile_h / 2, 0);

        set_entity_color(platform, (irand() % 255) << 16 | (irand() % 255) << 8 | (irand() % 255));
      }
    }
  }
  EndTextureMode();

  return cache;
}

resc_tile_flag grid_cell_get_flag(grid_cell *cell, resource_manager *rm) {
  return resource_get_tile_flag(rm, cell->tile_text_id);
}

void grid_cell_set_tile_id(grid_cell *cell, uint64_t tile_id) { cell->tile_text_id = tile_id; }
