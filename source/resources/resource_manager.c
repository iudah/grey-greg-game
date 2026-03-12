#include "resource_manager.h"

#include <grey_constants.h>
#include <ilist.h>
#include <raylib_glue.h>
#include <zot.h>

#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

#ifdef WIN32
#include <windows.h>
#endif

#ifdef __linux__
#include <limits.h>
#include <unistd.h>
#endif

struct tile {
  Rectangle src_rect;
  uint32_t texture_id;
  collision_flag flag;
  uint32_t coll_layer;
  uint32_t coll_mask;
};

struct resource_manager {
  ilist *textures;
  ilist *tiles;
};

char *_game_app_path;

char *game_app_path() {
  if (_game_app_path) {
    return _game_app_path;
  }
  size_t count = 1024;
  _game_app_path = zmalloc(sizeof(*_game_app_path) * count);
#ifdef WIN32
  GetModuleFileName(NULL, _game_app_path, count);
  count = strlen(_game_app_path);
#elif defined(__linux__)
  count = readlink("/proc/self/exe", _game_app_path, count);
  if (count == -1) {
    return NULL;
  }
#elif defined(__APPLE__)
  if (_NSGetExecutablePath(_game_app_path, &count) != 0) {
    return NULL;
  }
  count = strlen(_game_app_path);
#endif
  _game_app_path[count] = '\0';
  _game_app_path = realloc(_game_app_path, sizeof(*_game_app_path) * (count + 1));
  return _game_app_path;
}

resource_manager *resource_manager_create() {
  resource_manager *mgr = zmalloc(sizeof(*mgr));
  mgr->textures = ilist_create(sizeof(Texture2D));
  return mgr;
}

uint32_t resource_load_texture(resource_manager *mgr, const char *fpath) {
  char path[1024];
  snprintf(path, sizeof(path), "%s/../assets/%s", game_app_path(), fpath);
  Texture2D texture = LoadTexture(path);
  ilist_append(mgr->textures, &texture);
  return ilist_count(mgr->textures) - 1;
}

Texture2D resource_get_texture(resource_manager *mgr, uint32_t texture_id) {
  return *(Texture2D *)ilist_get(mgr->textures, texture_id);
}

uint32_t resource_make_tile(resource_manager *mgr, uint32_t texture_id, uint32_t x, uint32_t y,
                            uint32_t tile_width, uint32_t tile_height, collision_flag flag,
                            uint32_t coll_layer, uint32_t coll_mask) {
  if (!mgr->tiles) {
    mgr->tiles = ilist_create(sizeof(struct tile));
  }
  struct tile tile = {.src_rect = {x, y, tile_width, tile_height},
                      .texture_id = texture_id,
                      .flag = flag,
                      .coll_layer = coll_layer,
                      .coll_mask = coll_mask};

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

bool resource_get_tile_flag(resource_manager *mgr, uint32_t tile_id, collision_flag *flag) {
  struct tile *tile = ilist_get(mgr->tiles, tile_id);
  if (!tile)
    return false;
  else {
    *flag = tile->flag;
    return true;
  }
}

bool resource_get_tile_coll_layer(resource_manager *mgr, uint32_t tile_id, uint32_t *coll_layer) {
  struct tile *tile = ilist_get(mgr->tiles, tile_id);
  if (!tile)
    return false;
  else {
    *coll_layer = tile->coll_layer;
    return true;
  }
}

bool resource_get_tile_coll_mask(resource_manager *mgr, uint32_t tile_id, uint32_t *coll_mask) {
  struct tile *tile = ilist_get(mgr->tiles, tile_id);
  if (!tile)
    return false;
  else {
    *coll_mask = tile->coll_mask;
    return true;
  }
}
