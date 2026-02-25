#ifndef VERLET_COMPONENTS_H
#define VERLET_COMPONENTS_H

#include "component.h"

COMPONENT_DEFINE(verlet, {
  struct vec4_st  *acceleration;
});

#endif