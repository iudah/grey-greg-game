#include "gravity_system.h"

#include <game_logic.h>

#include "entity.h"
#include "force_component.h"
#include "mass_component.h"

float GRAVITATIONAL_ACCELERATION = (9.8f);

bool set_gravitational_acceleration(float acceleration) {
  if (acceleration < GREY_ZERO) return false;
  GRAVITATIONAL_ACCELERATION = acceleration;
  return true;
}

float get_gravitational_acceleration() { return GRAVITATIONAL_ACCELERATION; }

void gravity_system_update(game_logic *logic, float delta_time) {
  for (uint32_t i = 0; i < mass_component->set.count; ++i) {
    entity e = get_entity((struct generic_component *)mass_component, i);
    float *mass = get_mass(e);

    if (!mass) continue;

    if (*mass < GREY_ZERO) continue;

    add_force(e, (float[]){0, GRAVITATIONAL_ACCELERATION * *mass, 0, 0});
  }
}