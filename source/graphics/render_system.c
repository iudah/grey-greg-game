#include "render_system.h"

#include <input_system.h>
#include <inttypes.h>
#include <math.h>
#include <raylib_glue.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zot.h>

#include "aabb_component.h"
#include "component.h"
#include "entity.h"
#include "grid_component.h"
#include "position_component.h"
#include "render_component.h"
#include "rotation_component.h"
#include "scale_component.h"
#include "velocity_component.h"

uint64_t count = 0;
char *frame = 0;
void ppm_render() {
  const int width = SCREEN_X;
  const int height = SCREEN_Y;

  if (!frame)
    frame = zcalloc(width * height * 3, sizeof(*frame));
  else
    memset(frame, 0xff, width * height * 3);

  for (uint32_t i = 0; i < aabb_component->set.count; i++) {
    entity entity = get_entity(aabb_component, i);
    struct vec4_st *pos = get_position(entity);
    struct vec4_st *coll_extent = get_collision_extent(entity);
    struct vec4_st *color = get_color(entity);

    if (!pos || !coll_extent || !color) continue;

    struct vec4_st extent = {coll_extent->x, coll_extent->y, 0, 0};
    for (int64_t x = (int64_t)(pos->x - extent.x); x < (int64_t)(pos->x + extent.x); ++x) {
      if (x < 0 || x >= width) continue;

      for (int64_t y = (int64_t)(pos->y - extent.y); y < (int64_t)(pos->y + extent.y); ++y) {
        if (y < 0 || y >= height) continue;

        char *pxl = &frame[(uint32_t)(y * width * 3 + x * 3)];
        pxl[0] = color->x;
        pxl[1] = color->y;
        pxl[2] = color->z;
      }
    }
  }

  if (count >= 400) exit(0);
  LOG("                            `tmp/img_%08" PRIu64 ".ppm`", count);

  char path[255];
  snprintf(path, 255, "tmp/img_%08" PRIu64 ".ppm", count++);
  FILE *f = fopen(path, "wb");

  if (0) {
    LOG("`tmp/img_%08" PRIu64 ".ppm` could not be open.", count - 1);
  } else {
    fprintf(f,
            "P6\n"
            "%d %d\n"
            "255\n",
            width, height);
    fwrite(frame, sizeof(*frame), width * height * 3, f);
    fclose(f);
  }
}

void render_controller(controller_config *controller) {
#ifndef NO_RAYLIB
  Rectangle *pad = controller->pad;
  Circle *stick = controller->stick;
  DrawRectangleRec(pad[PAD_A], BLUE);
  DrawRectangleRec(pad[PAD_B], BLUE);
  DrawRectangleRec(pad[PAD_O], BLUE);
  DrawRectangleRec(pad[PAD_X], BLUE);
  DrawCircleLines(stick[STICK_BASE].x, stick[STICK_BASE].y, stick[STICK_BASE].radius, BLUE);
  DrawCircle(stick[STICK_KNOB].x, stick[STICK_KNOB].y, stick[STICK_KNOB].radius, Fade(BLUE, 0.4));
#endif
}

void render_grid(game_logic *logic, grid *g) {
  RenderTexture2D *cache = grid_get_grid_cache(g);

  if (cache == NULL) {
    cache = grid_bake(g, game_logic_get_resource_manager(logic));
    grid_set_grid_cache(g, cache);
  }

  if (cache) {
    Rectangle src = {0, 0, (float)cache->texture.width, (float)-cache->texture.height};
    Rectangle dest = {0, 0, (float)cache->texture.width, (float)cache->texture.height};
    DrawTexturePro(cache->texture, src, dest, (Vector2){0, 0}, 0.0f, WHITE);
  }
}

#ifndef NO_RAYLIB
void raylib_render(game_logic *logic) {
  for (uint32_t i = 0; i < render_component->set.count; ++i) {
    entity e = render_component->set.dense[i];
    if (has_component(e, (struct generic_component *)grid_component)) {
      render_grid(logic, grid_component_get_grid(e));
      continue;
    }

#if 1
    uint32_t aabb_i;
    if (!component_get_dense_id((struct generic_component *)aabb_component, e, &aabb_i)) continue;

    uint32_t pos_i;
    if (!component_get_dense_id((struct generic_component *)position_component, e, &pos_i))
      continue;

    struct vec4_st *interp_pos = get_interpolated_position(e);
    struct vec4_st *coll_extent = get_collision_extent(e);
    struct vec4_st *color = get_color(e);
    if (!color || !coll_extent||!interp_pos) continue;

    DrawRectangle(interp_pos->x - coll_extent->x, interp_pos->y - coll_extent->y,
                  coll_extent->x * 2, coll_extent->y * 2,
                  CLITERAL(Color){color->x, color->y, color->z, 180});
#endif
  }
}
#endif

void render_scene(game_logic *logic, float interpolation_factor) {
  if (interpolation_factor - 1 > GREY_ZERO) interpolation_factor = 1;
  interpolate_positions(interpolation_factor);
#ifndef NO_RAYLIB
  raylib_render(logic);
#else
  // ppm_render();
#endif
}
