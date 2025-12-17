#include "verlet_component.h"
#include <zot.h>

struct verlet_component *verlet_component;

bool initialize_verlet_component() {
  verlet_component = zcalloc(1, sizeof(struct verlet_component));
  verlet_component->acceleration =
      zcalloc(MAX_NO_ENTITY, sizeof(*verlet_component->acceleration));
  return verlet_component != NULL &&
         initialize_component((struct generic_component *)verlet_component,
                              sizeof(struct vec4_st));
}
