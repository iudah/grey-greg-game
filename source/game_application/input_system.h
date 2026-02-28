#ifndef INPUT_SYSTEM_H
#define INPUT_SYSTEM_H

#include <grey_constants.h>
#include <game_logic.h>
#include <raylib_glue.h>
#ifdef RAYLIB_H
typedef struct Circle {
  float x;
  float y;
  float radius;
} Circle;

struct controller_config {
  Rectangle pad[PAD_COUNT];
  Circle stick[STICK_COUNT];
};

#endif

typedef struct controller_config controller_config;

void update_input_system(game_logic *logic);
void render_controller(controller_config *controller);

#endif