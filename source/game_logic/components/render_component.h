#ifndef RENDER_COMPONENT_COMPONENT_H
#define RENDER_COMPONENT_COMPONENT_H

#include <stdint.h>

#include "component_base.h"

#define SCREEN_X 960
#define SCREEN_Y 720

struct render_component {
  component_set set;
  struct {
    // struct vec4_st *scale;
    // struct vec4_st *rotation;
    struct vec4_st* color;
    struct vec4_st* interpolated_position;
  }* streams;
};

extern struct render_component* render_component;

bool initialize_render_component();
bool set_entity_color(entity e, uint32_t rgba);

#endif
