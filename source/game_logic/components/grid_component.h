#ifndef GRID_COMPONENT_H
#define GRID_COMPONENT_H

#include <grid.h>

#include "component_base.h"

struct grid_component {
  component_set set;
  struct {
    grid **map;
  } *streams;
};

extern struct grid_component *grid_component;

bool initialize_grid_component();
grid *grid_component_get_grid(entity e);
void grid_component_set_grid(entity e, grid *grid);

#endif
