#include "waypoint_component.h"
#include <zot.h>

struct waypoint_component *waypoint_component;

bool initialize_waypoint_component() {
  waypoint_component = zcalloc(1, sizeof(struct waypoint_component));

  bool component_intialized = initialize_component(
      (struct generic_component *)waypoint_component,
      (uint64_t[]){sizeof((*waypoint_component->streams->waypoint))},
      sizeof(*waypoint_component->streams) / sizeof(void *));

  waypoint_component->streams->waypoint =
      zcalloc(MAX_NO_ENTITY, sizeof(*waypoint_component->streams->waypoint));

  return waypoint_component != NULL && component_intialized;
}