#ifndef RENDER_COMPONENT_COMPONENT_H
#define RENDER_COMPONENT_COMPONENT_H

#include "component_base.h"
#include <stdint.h>

struct render_component {
  component_set set;
  // struct vec4_st *scale;
  // struct vec4_st *rotation;
  struct vec4_st *color;
};

extern struct render_component *render_component;

bool initialize_render_component();
bool set_entity_color(entity e, uint32_t rgba);

#endif
