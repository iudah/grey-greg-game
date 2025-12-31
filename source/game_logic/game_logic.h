#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

#include <stdbool.h>

#define GREY_ZERO (1e-5f)

extern bool is_2d;

void use_2d();
void interpolate_positions(float interpolation_factor);
void systems_update();
void compute_collisions();

#endif