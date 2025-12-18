#include "velocity_component.h"
#include <stdint.h>
#include <zot.h>

struct velocity_component *velocity_component;

bool initialize_velocity_component() {
  velocity_component = zcalloc(1, sizeof(struct velocity_component));

  bool component_intialized = initialize_component(
      (struct generic_component *)velocity_component,
      (uint64_t[]){sizeof(*velocity_component->streams->velocity),
                   sizeof(*velocity_component->streams->acceleration)},
      sizeof(*velocity_component->streams) / sizeof(void *));

  //   velocity_component->streams->velocity =
  //       zcalloc(MAX_NO_ENTITY,
  //       sizeof(*velocity_component->streams->velocity));
  //   velocity_component->streams->acceleration = zcalloc(
  //       MAX_NO_ENTITY, sizeof(*velocity_component->streams->acceleration));

  return velocity_component != NULL && component_intialized;
}
