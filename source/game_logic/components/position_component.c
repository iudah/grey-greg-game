#include "position_component.h"

#include <simd.h>
#include <zot.h>

struct position_component *position_component;

bool initialize_position_component() {
  position_component = zcalloc(1, sizeof(struct position_component));

  bool component_intialized =
      initialize_component((struct generic_component *)position_component,
                           (uint64_t[]){
                               sizeof(*position_component->streams->position),
                               sizeof(*position_component->streams->previous_position)  //,

                               //  sizeof(*position_component->stream->curr_interp_position),
                               //  sizeof(*position_component->stream->prev_interp_position)
                           },
                           sizeof(*position_component->streams) / sizeof(void *));

  //   position_component->stream->position =
  //       zcalloc(MAX_NO_ENTITY,
  //       sizeof(*position_component->stream->position));
  //   position_component->stream->prev_position = zcalloc(
  //       MAX_NO_ENTITY, sizeof(*position_component->stream->prev_position));

  return position_component != NULL && component_intialized;
}

bool set_position(entity e, float *position) {
  float x = position[0];
  float y = position[1];
  float z = position[2];

  struct vec4_st *pos = get_position(e);
  struct vec4_st *prev_pos = get_previous_position(e);

  if (!pos || !prev_pos) {
    return false;
  }

  pos->x = x;
  pos->y = y;
  pos->z = z;

  prev_pos->x = x;
  prev_pos->y = y;
  prev_pos->z = z;

  return true;
}

bool snapshot_positions () {
  if (!position_component->streams->position || !position_component->streams->previous_position) {
    return false;
  }

  auto prev_pos = position_component->streams->previous_position;
  auto pos = position_component->streams->position;

  assert(position_component->set.count <= position_component->set.dense_capacity);

  memcpy(prev_pos, pos, position_component->set.count * sizeof(*prev_pos));

  return true;
}
