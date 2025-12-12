#include "aabb_component.h"
#include "component.h"
#include "position_component.h"
#include "render_component.h"
#include "rotation_component.h"
#include "scale_component.h"
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <zot.h>

void render_system_update() {
  struct vec4_st *rot = rotation_component->rotation;
  struct vec4_st *scale = scale_component->scale;

  for (uint32_t i = 0; i < scale_component->set.count; ++i) {
    // compute transform matrix
  }
}

uint64_t count = 0;
char *frame = 0;
void render() {
  const int width = 240;
  const int height = 320;

  if (!frame)
    frame = zcalloc(width * height * 3, sizeof(*frame));
  else
    memset(frame, 0xff, width * height * 3);

  struct vec4_st *extent = aabb_component->extent;
  entity *e = aabb_component->set.dense;
  struct vec4_st *position = position_component->curr_position;

  for (uint32_t i = 0; i < aabb_component->set.count; i++) {
    uint32_t j = position_component->set.sparse[e[i].id];

    if (!has_component(e[i], (struct generic_component *)render_component))
      continue;

    uint32_t r_idx = render_component->set.sparse[e[i].id];

    struct vec4_st *pos = &position[j];
    for (int64_t x = (int64_t)(pos->x - extent[i].x);
         x < (int64_t)(pos->x + extent[i].x); ++x) {

      if (x < 0 || x >= width)
        continue;

      for (int64_t y = (int64_t)(pos->y - extent[i].y);
           y < (int64_t)(pos->y + extent[i].y); ++y) {

        if (y < 0 || y >= height)
          continue;

        char *pxl = &frame[(uint32_t)(y * width * 3 + x * 3)];
        pxl[0] = render_component->color[r_idx].x;
        pxl[1] = render_component->color[r_idx].y;
        pxl[2] = render_component->color[r_idx].z;
      }
    }
  }

  char path[255];
  snprintf(path, 255, "tmp/img_%08" PRIu64 ".ppm", count++);
  FILE *f = fopen(path, "wb");

  fprintf(f,
          "P6\n"
          "%d %d\n"
          "255\n",
          width, height);
  fwrite(frame, sizeof(*frame), width * height * 3, f);
  fclose(f);
}
