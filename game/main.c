#include <game_main.h>

#include "world.h"

int game_main(void *);

int main() {
  set_game_screen_config(720, 360, "Grey Greg", 60);
  game_main(init_world);
}