#ifndef GRAVITY_SYSTEM_H
#define GRAVITY_SYSTEM_H

#include "entity.h"
#include "game_logic.h"

void gravity_system_update(game_logic *logic, float delta_time);
bool set_gravitational_acceleration(float acceleration);
float get_gravitational_acceleration();

#endif