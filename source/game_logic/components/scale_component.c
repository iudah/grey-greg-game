#include "scale_component.h"
#include <zot.h>

struct scale_component *scale_component;

bool initialize_scale_component() {

  scale_component = zcalloc(1, sizeof(struct scale_component));

  bool component_intialized = initialize_component(
      (struct generic_component *)scale_component,
      (uint64_t[]){sizeof((*scale_component->streams->scale))},
      sizeof(*scale_component->streams) / sizeof(void *));

  scale_component->streams->scale =
      zcalloc(MAX_NO_ENTITY, sizeof(*scale_component->streams->scale));

  return scale_component != NULL && component_intialized;
}