#include <grid.h>
#include <raylib_glue.h>
#include <stdint.h>
#include <zot.h>

struct grid_cell {
  int8_t y_offset;
  uint32_t tile_text_id;
  bool solid;
};

struct grid {
  RenderTexture2D *cache;
  struct grid_cell *grid;  // flattened grid
  uint64_t x, y;
};

grid *grid_create(uint64_t width, uint64_t height) {
  grid *g = zmalloc(sizeof(*g));
  g->grid = zcalloc(width * height, sizeof(*g->grid));

  g->cache = 0;
  g->x = width;
  g->y = height;

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
  int tile_size = 16;
  RenderTexture2D *cache = zmalloc(sizeof(RenderTexture2D));
  *cache = LoadRenderTexture(grid->x * tile_size, grid->y * tile_size);

  BeginTextureMode(*cache);
  ClearBackground(BLANK);

  for (uint32_t x = 0; x < grid->x; ++x) {
    for (uint32_t y = 0; y < grid->y; ++y) {
      grid_cell *cell = grid_get_cell(grid, x, y);

      if (cell->tile_text_id == 0) continue;

      Vector2 position = {x * tile_size, y * tile_size};

      Texture2D *tex = resource_get_tile_texture(resc_mgr, cell->tile_text_id);
      if (!tex) continue;

      Rectangle *rect = resource_get_tile_rect(resc_mgr, cell->tile_text_id);
      if (!rect) continue;

      DrawTextureRec(*tex, *rect, position, WHITE);
    }
  }
  EndTextureMode();

  return cache;
}
