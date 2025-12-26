#ifndef MASS_COMPONENT_H
#define MASS_COMPONENT_H

#include "component_base.h"

struct mass_component
{
    component_set set;
    struct
    {
        float *mass;
    } *stream;
};

extern struct mass_component *mass_component;

bool initialize_mass_component();
float get_mass(entity e);
bool set_mass(entity e, float mass);
static inline bool set_entity_mass(entity e, float mass) { return set_mass(e, mass); }

#endif