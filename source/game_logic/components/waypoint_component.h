#ifndef WAYPOINT_COMPONENT_H
#define WAYPOINT_COMPONENT_H

#include "component_base.h"

struct waypoint_component {
  component_set set;
  struct {
    struct vec4_st* waypoint;
  }* streams;
};

extern struct waypoint_component* waypoint_component;

bool initialize_waypoint_component();

#endif