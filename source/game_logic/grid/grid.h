#ifndef GRID_H
#define GRID_H

#include <raylib_glue.h>
#include <resource_manager.h>
#include <stdint.h>

typedef struct grid grid;
typedef struct grid_cell grid_cell;

grid *grid_create(uint64_t width, uint64_t height);
void grid_destroy(grid *g);
grid_cell *grid_get_cell(grid *g, uint64_t x, uint64_t y);
void grid_get_grid_size(grid *g, uint64_t size[2]);
void grid_set_grid_cache(grid *g, RenderTexture2D *cache);
RenderTexture2D *grid_get_grid_cache(grid *g);
RenderTexture2D *grid_bake(grid *grid, resource_manager *resc_mgr);

#endif