#include "position_component.h"
#include <zot.h>

struct position_component *position_component;

bool initialize_position_component() {
  position_component = zcalloc(1, sizeof(struct position_component));
  position_component->position =
      zcalloc(MAX_NO_ENTITY, sizeof(*position_component->position));
  position_component->prev_position =
      zcalloc(MAX_NO_ENTITY, sizeof(*position_component->prev_position));

  return position_component != NULL &&
         initialize_component((struct generic_component *)position_component,
                              sizeof(struct vec4_st));
}
