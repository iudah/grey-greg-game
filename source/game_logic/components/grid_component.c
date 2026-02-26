#include "grid_component.h"

#include <stdint.h>
#include <zot.h>

struct grid_component *grid_component;

COMPONENT_STREAM_DEFINE(grid, { grid **map; });

bool initialize_grid_component() {
  grid_component = zcalloc(1, sizeof(struct grid_component));
  auto component = (struct generic_component *)grid_component;

  bool component_intialized =
      initialize_component(component, (uint64_t[]){sizeof(*grid_component->streams->map)},
                           sizeof(*grid_component->streams) / sizeof(void *));

  return grid_component != NULL && component_intialized;
}

grid *grid_component_get_grid(entity e) {
  uint32_t id;
  if (!component_get_dense_id((struct generic_component *)grid_component, e, &id)) return NULL;

  return grid_component->streams->map[id];
}

void grid_component_set_grid(entity e, grid *grid) {
  uint32_t id;
  if (!component_get_dense_id((struct generic_component *)grid_component, e, &id)) return;

  grid_component->streams->map[id] = grid;
}
