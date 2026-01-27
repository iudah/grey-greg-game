#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

#include <event_system.h>
#include <stdbool.h>

typedef struct game_logic game_logic;

extern bool is_2d;

void use_2d();
void interpolate_positions(float interpolation_factor);
void systems_update(game_logic *logic, float delta_time);
void compute_collisions(game_logic *logic);

game_logic *game_logic_create();
void game_logic_update(game_logic *logic, float delta_time);
void game_logic_destroy(game_logic *logic);
event_system *game_logic_get_event_system(game_logic *logic);

#endif