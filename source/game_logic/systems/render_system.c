#include "aabb_component.h"
#include "component.h"
#include "position_component.h"
#include "render_component.h"
#include "render_system.h"
#include "rotation_component.h"
#include "scale_component.h"
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zot.h>

void render_system_update()
{
  struct vec4_st *rot = rotation_component->streams->rotation;
  struct vec4_st *scale = scale_component->streams->scale;

  for (uint32_t i = 0; i < scale_component->set.count; ++i)
  {
    // compute transform matrix
  }
}

uint64_t count = 0;
char *frame = 0;
void render()
{
  const int width = SCREEN_X;
  const int height = SCREEN_Y;

  if (!frame)
    frame = zcalloc(width * height * 3, sizeof(*frame));
  else
    memset(frame, 0xff, width * height * 3);

  struct vec4_st *extent = aabb_component->streams->extent;
  entity *e = aabb_component->set.dense;
  // todo: use render position
  struct vec4_st *position = position_component->stream->position;

  for (uint32_t i = 0; i < aabb_component->set.count; i++)
  {
    uint32_t j;
    if (!component_get_dense_id((struct generic_component *)position_component, e[i], &j))
      continue;

    uint32_t r_idx;
    if (!component_get_dense_id((struct generic_component *)render_component, e[i], &r_idx))
      continue;

    struct vec4_st *pos = &position[j];
    for (int64_t x = (int64_t)(pos->x - extent[i].x);
         x < (int64_t)(pos->x + extent[i].x); ++x)
    {

      if (x < 0 || x >= width)
        continue;

      for (int64_t y = (int64_t)(pos->y - extent[i].y);
           y < (int64_t)(pos->y + extent[i].y); ++y)
      {

        if (y < 0 || y >= height)
          continue;

        char *pxl = &frame[(uint32_t)(y * width * 3 + x * 3)];
        pxl[0] = render_component->streams->color[r_idx].x;
        pxl[1] = render_component->streams->color[r_idx].y;
        pxl[2] = render_component->streams->color[r_idx].z;
      }
    }
  }

  if (count >= 100)
    exit(0);
  LOG("                            `tmp/img_%08" PRIu64 ".ppm`", count);

  char path[255];
  snprintf(path, 255, "tmp/img_%08" PRIu64 ".ppm", count++);
  FILE *f = fopen(path, "wb");

  if (0)
  {
    LOG("`tmp/img_%08" PRIu64 ".ppm` could not be open.", count - 1);
  }
  else
  {
    fprintf(f,
            "P6\n"
            "%d %d\n"
            "255\n",
            width, height);
    fwrite(frame, sizeof(*frame), width * height * 3, f);
    fclose(f);
  }
}
