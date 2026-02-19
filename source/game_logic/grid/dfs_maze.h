#ifndef DFS_MAZE_H
#define DFS_MAZE_H

#include <stdint.h>

bool dfs_generate_maze(struct wfc_state *grid, uint32_t width, uint32_t height, int x0, int y0,
                       wfc_atlas *atlas, resource_manager *mgr);

#endif