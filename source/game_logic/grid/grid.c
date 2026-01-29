#include <grid.h>
#include <stdint.h>
#include <zot.h>

struct grid_cell {
  int8_t y_offset;
  uint8_t tile_text_id;
  bool solid;
};

struct grid {
  struct grid_cell *grid;  // flattened grid
  uint64_t x, y;
};

grid *grid_create(uint64_t width, uint64_t height) {
  grid *g = zmalloc(sizeof(*g));
  g->grid = zcalloc(width * height, sizeof(*g->grid));

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
