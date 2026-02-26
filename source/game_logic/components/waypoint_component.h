#ifndef WAYPOINT_COMPONENT_H
#define WAYPOINT_COMPONENT_H

#include "component_base.h"

COMPONENT_DEFINE(waypoint);

bool initialize_waypoint_component();
struct vec4_st *get_waypoint(entity e);
bool set_entity_waypoint(entity e, float x, float y, float z);
static inline bool set_waypoint(entity e, float *waypoint){return set_entity_waypoint(e, waypoint[0], waypoint[1], waypoint[2]);}
#endif