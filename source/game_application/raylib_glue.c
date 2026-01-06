#include "raylib_glue.h"

#include <raylib.h>
#include <stdint.h>

void init_window(uint32_t screen_width, uint32_t screen_height, char* title) {
  return InitWindow(screen_width, screen_height, title);
}

void set_FPS(uint32_t FPS) { return SetTargetFPS(FPS); }
bool window_should_close() { return WindowShouldClose(); }
void close_window() { return CloseWindow(); }
