#ifndef WAYPOINT_COMPONENT_H
#define WAYPOINT_COMPONENT_H

#include "component.h"

struct waypoint_component {
  component_set set;
  struct vec4_st {
    float x, y, z, w;
  } *waypoint;
};

extern struct waypoint_component *waypoint_component;

bool initialize_waypoint_component();

#endif