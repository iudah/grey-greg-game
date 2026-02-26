#include "waypoint_component.h"

#include <zot.h>

#include "component_base.h"

struct waypoint_component *waypoint_component;

COMPONENT_STREAM_DEFINE(waypoint, { struct vec4_st *waypoint; });

struct vec4_st *get_waypoint(entity e) { return COMPONENT_GET(waypoint, e, waypoint); }

bool set_waypoint(entity e, float x, float y, float z) {
  struct vec4_st *waypoint = get_waypoint(e);
  if (!waypoint) return false;

  waypoint->x = x;
  waypoint->y = y;
  waypoint->z = z;
  return true;
}

bool set_entity_waypoint(entity e, float x, float y, float z) {
  struct vec4_st *waypoint = get_waypoint(e);
  if (!waypoint) return false;

  waypoint->x = x;
  waypoint->y = y;
  waypoint->z = z;
  return true;
}

bool initialize_waypoint_component() {
  waypoint_component = zcalloc(1, sizeof(struct waypoint_component));

  bool component_intialized =
      initialize_component((struct generic_component *)waypoint_component,
                           (uint64_t[]){sizeof((*waypoint_component->streams->waypoint))},
                           sizeof(*waypoint_component->streams) / sizeof(void *));

  return waypoint_component != NULL && component_intialized;
}