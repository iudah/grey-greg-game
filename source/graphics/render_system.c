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

  struct vec4_st *extent = aabb_component->streams->extent;
  entity *e = aabb_component->set.dense;
  struct vec4_st *position = render_component->streams->interpolated_position;

  for (uint32_t i = 0; i < aabb_component->set.count; i++) {
    uint32_t j;
    if (!component_get_dense_id((struct generic_component *)position_component, e[i], &j)) continue;

    uint32_t r_idx;
    if (!component_get_dense_id((struct generic_component *)render_component, e[i], &r_idx))
      continue;

    struct vec4_st *pos = &position[j];
    for (int64_t x = (int64_t)(pos->x - extent[i].x); x < (int64_t)(pos->x + extent[i].x); ++x) {
      if (x < 0 || x >= width) continue;

      for (int64_t y = (int64_t)(pos->y - extent[i].y); y < (int64_t)(pos->y + extent[i].y); ++y) {
        if (y < 0 || y >= height) continue;

        char *pxl = &frame[(uint32_t)(y * width * 3 + x * 3)];
        pxl[0] = render_component->streams->color[r_idx].x;
        pxl[1] = render_component->streams->color[r_idx].y;
        pxl[2] = render_component->streams->color[r_idx].z;
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
    Rectangle src = {0, 0, (float)grid_get_grid_cache(g)->texture.width,
                     (float)-grid_get_grid_cache(g)->texture.height};
    Rectangle dest = {0, 0, (float)get_screen_width(), (float)get_screen_height()};
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

#if 0
    uint32_t aabb_i;
    if (!component_get_dense_id((struct generic_component *)aabb_component, e, &aabb_i)) continue;

    uint32_t pos_i;
    if (!component_get_dense_id((struct generic_component *)position_component, e, &pos_i))
      continue;

    DrawRectangle(render_component->streams->interpolated_position[pos_i].x -
                      aabb_component->streams->extent[aabb_i].x,
                  render_component->streams->interpolated_position[pos_i].y -
                      aabb_component->streams->extent[aabb_i].y,
                  aabb_component->streams->extent[aabb_i].x * 2,
                  aabb_component->streams->extent[aabb_i].y * 2,
                  CLITERAL(Color){render_component->streams->color[i].x,
                                  render_component->streams->color[i].y,
                                  render_component->streams->color[i].z, 255});
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
