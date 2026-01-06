#ifndef RAYLIB_GLUE_H
#define RAYLIB_GLUE_H

#include <stdbool.h>
#include <stdint.h>

void init_window(uint32_t screen_width, uint32_t screen_height, char* title);
void set_FPS(uint32_t FPS);
bool window_should_close();
void close_window();

#endif