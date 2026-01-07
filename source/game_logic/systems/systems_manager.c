#include "systems_manager.h"

#include <zot.h>

#include "event_system.h"
#include "game_logic.h"

#define MAX_SYSTEM 32

typedef struct {
  system_update_fn_t* system;
  uint32_t no_system;
  uint32_t system_cap;
} system_manager;

static system_manager manager;

void static __attribute__((constructor(202))) init() {
  manager.system_cap = MAX_SYSTEM;
  manager.system = zmalloc(manager.system_cap * sizeof(*manager.system));
  manager.no_system = 0;
}

bool register_system_update(system_update_fn_t system) {
  if (manager.no_system < manager.system_cap) {
    manager.system[manager.no_system++] = system;
    return true;
  }

  return false;
}

void systems_update() {
  for (uint32_t i = 0; i < manager.no_system; ++i) {
    manager.system[i]();
  }
  event_default_broadcast();
}