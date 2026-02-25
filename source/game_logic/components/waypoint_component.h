#ifndef WAYPOINT_COMPONENT_H
#define WAYPOINT_COMPONENT_H

#include "component_base.h"

COMPONENT_DEFINE(waypoint, { struct vec4_st *waypoint; });
static inline struct vec4_st *get_waypoint(entity e) {
  return COMPONENT_GET(waypoint_component, e, waypoint);
}

#endif