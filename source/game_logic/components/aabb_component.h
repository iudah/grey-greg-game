#ifndef AABB_COMPONENT_H
#define AABB_COMPONENT_H

#include "component_base.h"

COMPONENT_DEFINE(aabb);

bool initialize_aabb_component() ;struct vec4_st *get_collision_extent(entity e);
float *get_collision_radius(entity e);
#endif
