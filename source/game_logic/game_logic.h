#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

#define GREY_ZERO (1e-6f)

void interpolate_positions(float interpolation_factor);
void systems_update();
void compute_swept_aabb_collision();

#endif