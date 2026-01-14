#ifndef INPUT_SYSTEM_H
#define INPUT_SYSTEM_H

typedef struct Circle {
  float x;
  float y;
  float radius;
} Circle;

void update_input_system();
void render_controller();

#endif