#include "mass_component.h"

#include <simd.h>
#include <string.h>
#include <zot.h>

struct mass_component* mass_component;

bool initialize_mass_component() {
  mass_component = zcalloc(1, sizeof(struct mass_component));

  bool component_intialized =
      initialize_component((struct generic_component*)mass_component,
                           (uint64_t[]){sizeof(*mass_component->stream->mass)},
                           sizeof(*mass_component->stream) / sizeof(void*));

  return mass_component != NULL && component_intialized;
}

float get_mass(entity e) {
  uint32_t dense_idx;
  if (!component_get_dense_id((struct generic_component*)mass_component, e,
                              &dense_idx))
    return 0;

  return mass_component->stream->mass[dense_idx];
}

bool set_mass(entity e, float mass) {
  uint32_t dense_idx;
  if (!component_get_dense_id((struct generic_component*)mass_component, e,
                              &dense_idx))
    return false;

  mass_component->stream->mass[dense_idx] = mass;
  return true;
}
