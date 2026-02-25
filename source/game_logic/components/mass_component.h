#ifndef MASS_COMPONENT_H
#define MASS_COMPONENT_H

#include "component_base.h"

COMPONENT_DEFINE(mass, { float *mass; });

static inline
float *get_mass(entity e) { return COMPONENT_GET(mass_component, e, mass); }
bool set_mass(entity e, float mass);
static inline bool set_entity_mass(entity e, float mass) { return set_mass(e, mass); }

#endif