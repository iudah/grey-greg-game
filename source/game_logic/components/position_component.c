#include "position_component.h"
#include <simd.h>
#include <zot.h>

struct position_component *position_component;

bool initialize_position_component()
{
  position_component = zcalloc(1, sizeof(struct position_component));

  bool component_intialized = initialize_component(
      (struct generic_component *)position_component,
      (uint64_t[]){
          sizeof(*position_component->stream->position),
          sizeof(*position_component->stream->prev_position) //,
                                                             //  sizeof(*position_component->stream->curr_interp_position),
                                                             //  sizeof(*position_component->stream->prev_interp_position)
      },
      sizeof(*position_component->stream) / sizeof(void *));

  //   position_component->stream->position =
  //       zcalloc(MAX_NO_ENTITY,
  //       sizeof(*position_component->stream->position));
  //   position_component->stream->prev_position = zcalloc(
  //       MAX_NO_ENTITY, sizeof(*position_component->stream->prev_position));

  return position_component != NULL && component_intialized;
}

struct vec4_st *get_position(entity e)
{
  if (!has_component(e, (struct generic_component *)position_component))
    return NULL;

  uint32_t j = position_component->set.sparse[e.id];

  return &position_component->stream->position[j];
}

struct vec4_st *get_previous_position(entity e)
{
  if (!has_component(e, (struct generic_component *)position_component))
    return NULL;

  uint32_t j = position_component->set.sparse[e.id];

  return &position_component->stream->prev_position[j];
}

bool set_position(entity e, float *position)
{

  float x = position[0];
  float y = position[1];
  float z = position[2];

  if (!has_component(e, (struct generic_component *)position_component))
    return false;

  uint32_t j = position_component->set.sparse[e.id];

  position_component->stream->position[j].x = x;
  position_component->stream->position[j].y = y;
  position_component->stream->position[j].z = z;

  position_component->stream->prev_position[j].x = x;
  position_component->stream->prev_position[j].y = y;
  position_component->stream->prev_position[j].z = z;

  return true;
}
