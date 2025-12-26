#include "gravity_system.h"
#include "force_component.h"
#include "mass_component.h"
#include "entity.h"

void gravity_system_update()
{
#define GRAVITATIONAL_ACCEL (9.8f)
    float *mass = mass_component->stream->mass;
    for (uint32_t i = 0; i < mass_component->set.count; ++i)
    {
        auto e = get_entity((struct generic_component *)mass_component, i);
        auto mass = get_mass(e);

        if (mass < GREY_ZERO)
            continue;

        add_force(e, (float[]){0, GRAVITATIONAL_ACCEL * mass, 0, 0});
    }
}