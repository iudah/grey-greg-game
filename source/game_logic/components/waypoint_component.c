#include "waypoint_component.h"
#include <zot.h>

struct waypoint_component *waypoint_component;

bool initialize_waypoint_component() {
  waypoint_component = zcalloc(1, sizeof(struct waypoint_component));
  waypoint_component->waypoint =
      zcalloc(MAX_NO_ENTITY, sizeof(*waypoint_component->waypoint));
  return waypoint_component != NULL &&
         initialize_component((struct generic_component *)waypoint_component,
                              sizeof(struct vec4_st));
}