#include "game_logic.h"

void ai_system_update();
void physics_system_update();
void render_system_update();

void systems_update() {
  ai_system_update();
  physics_system_update();
  render_system_update();
}