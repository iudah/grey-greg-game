#include "resource_manager.h"

#include <ilist.h>
#include <raylib_glue.h>
#include <zot.h>

struct tile {
  Rectangle src_rect;
  uint32_t texture_id;
  resc_tile_flag flag;
};

struct resource_manager {
  ilist *textures;
  ilist *tiles;
};

resource_manager *resource_manager_create() {
  resource_manager *mgr = zmalloc(sizeof(*mgr));
  mgr->textures = ilist_create(sizeof(Texture2D));
  return mgr;
}

uint32_t resource_load_texture(resource_manager *mgr, const char *fpath) {
  Texture2D texture = LoadTexture(fpath);
  ilist_append(mgr->textures, &texture);
  return ilist_count(mgr->textures) - 1;
}

Texture2D resource_get_texture(resource_manager *mgr, uint32_t texture_id) {
  return *(Texture2D *)ilist_get(mgr->textures, texture_id);
}

uint32_t resource_make_tile(resource_manager *mgr, uint32_t texture_id, uint32_t x, uint32_t y,
                            uint32_t tile_width, uint32_t tile_height, resc_tile_flag flag) {
  if (!mgr->tiles) {
    mgr->tiles = ilist_create(sizeof(struct tile));
  }
  struct tile tile = {.src_rect = {x, y, tile_width, tile_height}, .texture_id = texture_id, flag};

  ilist_append(mgr->tiles, &tile);

  return ilist_count(mgr->tiles) - 1;
}

Rectangle *resource_get_tile_rect(resource_manager *mgr, uint32_t tile_id) {
  struct tile *tile = ilist_get(mgr->tiles, tile_id);

  if (!tile) return NULL;
  return tile->src_rect.width && tile->src_rect.height ? &tile->src_rect : NULL;
}

Texture2D *resource_get_tile_texture(resource_manager *mgr, uint32_t tile_id) {
  struct tile *tile = ilist_get(mgr->tiles, tile_id);

  if (!tile) return NULL;
  return ilist_get(mgr->textures, tile->texture_id);
}

resc_tile_flag resource_get_tile_flag(resource_manager *mgr, uint32_t tile_id) {
  struct tile *tile =ilist_get(mgr->tiles, tile_id);
  if(!tile)return TILE_UNKNOWN;
  else
  return tile->flag;
}
