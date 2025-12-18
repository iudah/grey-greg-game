#include "position_component.h"
#include <zot.h>

struct position_component *position_component;

bool initialize_position_component() {
  position_component = zcalloc(1, sizeof(struct position_component));

  bool component_intialized = initialize_component(
      (struct generic_component *)position_component,
      (uint64_t[]){sizeof(*position_component->stream->position),
                   sizeof(*position_component->stream->prev_position),
                   sizeof(*position_component->stream->curr_position),
                   sizeof(*position_component->stream->prev_timestep_pos)},
      sizeof(*position_component->stream) / sizeof(void *));

  //   position_component->stream->position =
  //       zcalloc(MAX_NO_ENTITY,
  //       sizeof(*position_component->stream->position));
  //   position_component->stream->prev_position = zcalloc(
  //       MAX_NO_ENTITY, sizeof(*position_component->stream->prev_position));

  return position_component != NULL && component_intialized;
}
