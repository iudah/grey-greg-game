#include "game_logic.h"

#include <event_system.h>
#include <process_manager.h>
#include <resource_manager.h>
#include <zot.h>

struct game_logic {
  event_system *event_system;
  process_manager *process_manager;
  resource_manager *resource_manager;
};

game_logic *game_logic_create() {
  game_logic *logic = zmalloc(sizeof(*logic));

  logic->event_system = event_system_create(0);
  logic->process_manager = process_manager_create();
  logic->resource_manager = resource_manager_create();

  return logic;
}
void game_logic_destroy(game_logic *logic) {
  process_manager_destroy(logic->process_manager);
  event_system_destroy(logic->event_system);

  zfree(logic);
}

void game_logic_update(game_logic *logic, float delta_time) {
  process_manager_update(logic->process_manager, delta_time);
  systems_update(logic, delta_time);
  event_system_update(logic->event_system);
}

event_system *game_logic_get_event_system(game_logic *logic) {
  if (!logic) return NULL;
  return logic->event_system;
}

resource_manager *game_logic_get_resource_manager(game_logic *logic) {
  if (!logic) return NULL;
  return logic->resource_manager;
}