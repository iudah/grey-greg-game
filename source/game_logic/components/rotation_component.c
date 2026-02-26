#include "rotation_component.h"

#include <stdint.h>
#include <zot.h>

#include "component_base.h"

struct rotation_component* rotation_component;

COMPONENT_STREAM_DEFINE(rotation, { struct vec4_st *rotation; });

bool initialize_rotation_component() {
  rotation_component = zcalloc(1, sizeof(struct rotation_component));

  bool component_intialized = initialize_component(
      (struct generic_component*)rotation_component,
      (uint64_t[]){sizeof(struct vec4_st)},
      sizeof(*rotation_component->streams) / sizeof(void*));

  return rotation_component != NULL && component_intialized;
}


