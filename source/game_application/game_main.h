#ifndef GAME_MAIN_H
#define GAME_MAIN_H

#include <stdint.h>

extern float TIMESTEP;

void set_game_screen_config(uint32_t screen_width, uint32_t screen_height, char *title,
                            uint32_t FPS);

#endif