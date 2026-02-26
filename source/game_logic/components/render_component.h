#ifndef RENDER_COMPONENT_COMPONENT_H
#define RENDER_COMPONENT_COMPONENT_H

#include <raylib_glue.h>
#include <stdint.h>

#include "component_base.h"

#define SCREEN_X get_screen_width()
#define SCREEN_Y get_screen_height()

COMPONENT_DEFINE(render);
bool initialize_render_component();
struct vec4_st *get_color(entity e) ;
bool set_entity_color(entity e, uint32_t rgba);
struct vec4_st *get_interpolated_position(entity e) ;

#endif
