#ifndef RENDER_COMPONENT_COMPONENT_H
#define RENDER_COMPONENT_COMPONENT_H

#include <raylib_glue.h>
#include <stdint.h>

#include "component_base.h"

#define SCREEN_X get_screen_width()
#define SCREEN_Y get_screen_height()

COMPONENT_DEFINE(render, {
  // struct vec4_st *scale;
  // struct vec4_st *rotation;
  struct vec4_st *color;
  struct vec4_st *interpolated_position;
});

static inline struct vec4_st *get_color(entity e) {
  return COMPONENT_GET(render_component, e, color);
}

bool set_entity_color(entity e, uint32_t rgba);

#endif
