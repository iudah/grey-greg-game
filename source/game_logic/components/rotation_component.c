#include "rotation_component.h"
#include "component_base.h"
#include <zot.h>

struct rotation_component *rotation_component;

bool initialize_rotation_component() {
  rotation_component = zcalloc(1, sizeof(struct rotation_component));
  return rotation_component != NULL &&
         initialize_component((struct generic_component *)rotation_component,
                              sizeof(struct vec4_st));
}
