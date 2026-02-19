#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <raylib_glue.h>

typedef struct resource_manager resource_manager;
typedef enum {
  TILE_NO_FLAG = 0,
  TILE_SOLID = 1 << 0,
  TILE_WALKABLE = 1 << 1,
  TILE_HAZARD = 1 << 2,
  TILE_CLIMBABLE = 1 << 3
} resc_tile_flag;

#if !HAS_WIN32_API
resource_manager *resource_manager_create();
uint32_t resource_load_texture(resource_manager *mgr, const char *fpath);
Texture2D resource_get_texture(resource_manager *mgr, uint32_t texture_id);
uint32_t resource_make_tile(resource_manager *mgr, uint32_t texture_id, uint32_t x, uint32_t y,
                            uint32_t tile_width, uint32_t tile_height, resc_tile_flag flag);
Rectangle *resource_get_tile_rect(resource_manager *mgr, uint32_t tile_id);
Texture2D *resource_get_tile_texture(resource_manager *mgr, uint32_t tile_id);
resc_tile_flag resource_get_tile_flag(resource_manager *mgr, uint32_t tile_id);

#endif
#endif
