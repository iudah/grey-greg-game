#ifndef MASS_COMPONENT_H
#define MASS_COMPONENT_H

#include "component_base.h"

COMPONENT_DEFINE(mass);

bool initialize_mass_component() ;float *get_mass(entity e);
bool set_mass(entity e, float mass);
static inline bool set_entity_mass(entity e, float mass) { return set_mass(e, mass); }

#endif