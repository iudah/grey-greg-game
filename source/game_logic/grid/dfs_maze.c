#include <component.h>
#include <irand.h>
#include <istack.h>
#include <strings.h>
#include <zot.h>

#include "grid.h"
#include "wave_fn_collapse.h"

bool cell_is_visited(uint64_t *visited, uint64_t idx) { return GET_BIT(visited, idx) != 0; }
void visit_cell(uint64_t *visited, uint64_t idx) { SET_BIT(visited, idx); }

void shuffle_direction_array(wfc_direction *directions) {
  for (uint8_t i = 0; i < 3; ++i) {
    uint8_t idx = irand() % (4 - i);
    if (directions[3 - i] == directions[idx]) continue;

    wfc_direction tmp = directions[3 - i];
    directions[3 - i] = directions[idx];
    directions[idx] = tmp;
  }
}

bool carve_cell_air(uint64_t *visited, wfc_atlas *atlas, uint32_t width, uint32_t height,
                    uint32_t x, uint32_t y, uint32_t air_height) {
  if (x >= width || y >= height) return false;

  for (uint32_t i = 0; i < air_height; ++i) {
    struct grid_coord coord = {.x = x, .y = y > i ? y - i : (height + y - i)};
    visit_cell(visited, coord.y * width + coord.x);
  }

  return true;
}

void carve_room_air(uint64_t *visited, wfc_atlas *atlas, uint32_t width, uint32_t height,
                    uint32_t cx, uint32_t cy, uint32_t rw, uint32_t rh) {
  int32_t start_x = (int32_t)cx - (int32_t)(rw / 2);
  int32_t start_y = (int32_t)cy - (int32_t)(rh / 2);

  for (uint32_t y = 0; y < rh; ++y) {
    for (uint32_t x = 0; x < rw; ++x) {
      int32_t px = start_x + (int32_t)x;
      int32_t py = start_y + (int32_t)y;

      if (px < 0) px += width;
      if (py < 0) py += height;
      if (px >= width) px -= width;
      if (py >= height) py -= height;

      // if (px >= 0 && px < (int32_t)width && py >= 0 && py < (int32_t)height)
      {
        visit_cell(visited, py * width + px);
      }
    }
  }
}

uint32_t count_air_column(uint64_t *visited, uint32_t width, uint32_t height, uint32_t x,
                          uint32_t y) {
  uint32_t col_height = 0;
  uint32_t up = y - 1;
  uint32_t dn = y + 1;

  while (up < height) {
#if 0
    if (!cell_is_visited(visited, up * width + x)) break;
    if (x + 1 < width && cell_is_visited(visited, up * width + (x + 1))) break;
    if (x > 0 && cell_is_visited(visited, up * width + (x - 1))) break;

    col_height++;
    up--;
#else
    if ((cell_is_visited(visited, up * width + x) &&
         (x + 1 < width && !cell_is_visited(visited, up * width + (x + 1)))) ||
        (cell_is_visited(visited, up * width + x) &&
         (x > 0 && !cell_is_visited(visited, up * width + (x - 1))))) {
      col_height++;
      up--;
    } else {
      break;
    }
#endif
  }

  while (dn < height) {
#if 0
    if (!cell_is_visited(visited, dn * width + x)) break;
    if (x + 1 < width && cell_is_visited(visited, dn * width + (x + 1))) break;
    if (x > 0 && cell_is_visited(visited, dn * width + (x - 1))) break;

    col_height++;
    dn++;
#else
    if ((cell_is_visited(visited, dn * width + x) &&
         (x + 1 < width && !cell_is_visited(visited, dn * width + (x + 1)))) ||
        (cell_is_visited(visited, dn * width + x) &&
         (x > 0 && !cell_is_visited(visited, dn * width + (x - 1))))) {
      col_height++;
      dn--;
    } else {
      break;
    }
#endif
  }

  return col_height;
}

bool dfs_generate_maze(struct wfc_state *grid, uint32_t width, uint32_t height, int x0, int y0,
                       wfc_atlas *atlas, resource_manager *mgr) {
  struct maze_node {
    struct grid_coord coord;
    wfc_direction dir[4];
    uint8_t next_dir_idx;
  };

  uint64_t *visited = zcalloc(required_uint64s(width * height), sizeof(*visited));

  istack *backtrack_stack = istack_create(sizeof(struct maze_node));

  if (y0 < 1) y0 = 1;

  wfc_direction base_directions[] = {NORTH, SOUTH, WEST, EAST};

  struct maze_node node = {.coord.x = x0, .coord.y = y0, .next_dir_idx = 0};
  memcpy(node.dir, base_directions, sizeof(base_directions));
  shuffle_direction_array(node.dir);
  istack_push(backtrack_stack, &node);

  uint32_t room_chance = 10;
  uint32_t min_room_size = 2;
  uint32_t max_room_size = 6;
  uint32_t min_air_height = 2;
  uint32_t max_air_height = 5;
  uint32_t x_step = 3;

  int32_t directions[][2] = {[NORTH] = {0, -max_air_height},
                             [SOUTH] = {0, max_air_height},
                             [WEST] = {-x_step, 0},
                             [EAST] = {x_step, 0}};

  while (istack_pop(backtrack_stack, &node)) {
    uint32_t x = node.coord.x, y = node.coord.y;

    uint32_t air_height = min_air_height + (irand() % (max_air_height - min_air_height));

    if (!cell_is_visited(visited, y * width + x)) {
      carve_cell_air(visited, atlas, width, height, x, y, air_height);

      bool has_tall_air_column =
          (node.dir[node.next_dir_idx] == NORTH || node.dir[node.next_dir_idx] == SOUTH) &&
          count_air_column(visited, width, height, x, y) >= (max_air_height + max_room_size / 2);

      if (has_tall_air_column || (irand() % 100) < room_chance) {
        uint32_t room_width = min_room_size + (irand() % (max_room_size - min_room_size + 1));
        uint32_t room_height = min_room_size + (irand() % (max_room_size - min_room_size + 1));

        carve_room_air(visited, atlas, width, height, x, y, room_width, room_height);
      }
      if (has_tall_air_column) continue;
    }

    int8_t dx = directions[node.dir[node.next_dir_idx]][0];
    int8_t dy = directions[node.dir[node.next_dir_idx]][1];
    int32_t neighbor_x = (int32_t)x + dx;
    int32_t neighbor_y = (int32_t)y + dy;

    node.next_dir_idx++;
    if (node.next_dir_idx < 4) istack_push(backtrack_stack, &node);
#if 1
    if (neighbor_x >= 0 && neighbor_x < (int32_t)width && neighbor_y >= 0 &&
        neighbor_y < (int32_t)height && !cell_is_visited(visited, neighbor_y * width + neighbor_x))
#else
    if (neighbor_x < 0) {
      neighbor_x += (int32_t)width;
    }
    if (neighbor_y < 0) {
      neighbor_y += (int32_t)height;
    }
    // We don't expect neighour_{x,y} to go up to 2 times {width, height} or more
    if (neighbor_x >= (int32_t)width) {
      neighbor_x -= (int32_t)width;
    }
    if (neighbor_y >= (int32_t)height) {
      neighbor_y -= (int32_t)height;
    }
    if (!cell_is_visited(visited, neighbor_y * width + neighbor_x))
#endif
    {
      int32_t step_x = (dx == 0) ? 0 : (dx > 0 ? 1 : -1);
      int32_t step_y = (dy == 0) ? 0 : (dy > 0 ? 1 : -1);

      uint32_t curr_x = x + step_x;
      uint32_t curr_y = y + step_y;

      while (curr_x != neighbor_x || curr_y != neighbor_y) {
        carve_cell_air(visited, atlas, width, height, curr_x, curr_y, air_height);

        if (!curr_x && step_x < 0) curr_x = width;
        if (!curr_y && step_y < 0) curr_y = height;
        curr_x += step_x;
        curr_y += step_y;
        if (curr_x >= width) curr_x -= width;
        if (curr_y >= height) curr_y -= height;
      }
      struct maze_node neighbor_node = {
          .coord.x = neighbor_x, .coord.y = neighbor_y, .next_dir_idx = 0};
      memcpy(neighbor_node.dir, base_directions, sizeof(base_directions));
      shuffle_direction_array(neighbor_node.dir);
      istack_push(backtrack_stack, &neighbor_node);
    }
  }

  istack_destroy(backtrack_stack);

  for (uint32_t y = 0; y < height; ++y) {
    for (uint32_t x = 0; x < width; ++x) {
      struct grid_coord origin = {x, y};
      if (cell_is_visited(visited, y * width + x)) {
        make_cell_empty(grid, atlas, mgr, width, height, origin);
      } else {
        make_cell_solid(grid, atlas, mgr, width, height, origin);
      }
    }
  }
  for (uint32_t y = 0; y < height; ++y) {
    for (uint32_t x = 0; x < width; ++x) {
      struct grid_coord origin = {x, y};
      if (!wfc_state_propagate(grid, atlas, width, height, origin)) {
        LOG("wfc contradiction during initial maze propagation at %d, %d", x, y);
        zfree(visited);
        return false;
      }
    }
  }

  zfree(visited);

  return true;
}
