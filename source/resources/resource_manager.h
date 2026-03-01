#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <grey_constants.h>
#include <raylib_glue.h>

typedef struct resource_manager resource_manager;

#if !HAS_WIN32_API
resource_manager *resource_manager_create();
uint32_t resource_load_texture(resource_manager *mgr, const char *fpath);
Texture2D resource_get_texture(resource_manager *mgr, uint32_t texture_id);
uint32_t resource_make_tile(resource_manager *mgr, uint32_t texture_id, uint32_t x, uint32_t y,
                            uint32_t tile_width, uint32_t tile_height, collision_flag flag,
                            uint32_t coll_layer, uint32_t coll_mask);
Rectangle *resource_get_tile_rect(resource_manager *mgr, uint32_t tile_id);
Texture2D *resource_get_tile_texture(resource_manager *mgr, uint32_t tile_id);
bool resource_get_tile_flag(resource_manager *mgr, uint32_t tile_id, collision_flag *flag);
bool resource_get_tile_coll_layer(resource_manager *mgr, uint32_t tile_id, uint32_t *coll_layer);
bool resource_get_tile_coll_mask(resource_manager *mgr, uint32_t tile_id, uint32_t *coll_mask);

#endif
#endif
