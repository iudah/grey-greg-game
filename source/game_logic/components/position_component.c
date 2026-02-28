#include "position_component.h"

#include <simd.h>
#include <string.h>
#include <zot.h>

#include "collision_component.h"

struct position_component *position_component;

COMPONENT_STREAM_DEFINE(position, {
  struct vec4_st *position;
  struct vec4_st *previous_position;
  // struct vec4_st *curr_interp_position;
  // struct vec4_st *prev_interp_position;
});

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

struct vec4_st *get_position(entity e) { return COMPONENT_GET(position, e, position); }
struct vec4_st *get_previous_position(entity e) {
  return COMPONENT_GET(position, e, previous_position);
}
bool set_entity_position(entity e, float x, float y, float z) {
  return set_position(e, (float[]){x, y, z});
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

  prev_pos->x = pos->x;
  prev_pos->y = pos->y;
  prev_pos->z = pos->z;

  pos->x = x;
  pos->y = y;
  pos->z = z;

  set_collision_spatial_dirty(e, true);

  return true;
}


