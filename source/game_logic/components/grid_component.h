#ifndef GRID_COMPONENT_H
#define GRID_COMPONENT_H

#include <grid.h>

#include "component_base.h"

COMPONENT_DEFINE(grid);

bool initialize_grid_component();
grid *grid_component_get_grid(entity e);
void grid_component_set_grid(entity e, grid *grid);

#endif
