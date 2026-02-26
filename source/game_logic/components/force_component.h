#ifndef FORCE_COMPONENT_H
#define FORCE_COMPONENT_H

#include "component_base.h"

COMPONENT_DEFINE(force);

bool initialize_force_component();
bool add_force(entity e, float *force);
struct vec4_st *get_force(entity e);
bool apply_force(entity e, float x, float y, float z);
void clear_forces();

#endif