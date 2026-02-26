#include "mass_component.h"

#include <simd.h>
#include <string.h>
#include <zot.h>

struct mass_component* mass_component;

COMPONENT_STREAM_DEFINE(mass, { float *mass; });

bool initialize_mass_component() {
  mass_component = zcalloc(1, sizeof(struct mass_component));

  bool component_intialized =
      initialize_component((struct generic_component*)mass_component,
                           (uint64_t[]){sizeof(*mass_component->streams->mass)},
                           sizeof(*mass_component->streams) / sizeof(void*));

  return mass_component != NULL && component_intialized;
}

float *get_mass(entity e){
  return COMPONENT_GET(mass, e, mass);
}

bool set_mass(entity e, float mass) {
  float* mass_ptr = get_mass(e);
  if (!mass_ptr) return false;
  *mass_ptr = mass;
  return true;
}
