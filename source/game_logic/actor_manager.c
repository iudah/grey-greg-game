#include "actor.h"
#include "entity.h"
#include <stdlib.h>
#include <zot.h>

typedef struct {
  entity *actors;
  uint64_t n_actors;
} actor_manager;

actor_manager *create_actor_manager() {
  actor_manager *manager = zcalloc(1, sizeof(actor_manager));
  manager->actors = zcalloc(MAX_NO_ENTITY, sizeof(entity));
  return manager;
}

bool destroy_actor_manager(actor_manager *manager) {
  for (uint64_t i = 0; i < manager->n_actors; ++i) {
    destroy_actor(manager->actors[i]);
  }
  zfree(manager->actors);
  zfree(manager);
  return true;
}

bool actor_manager_update(actor_manager *manager) {
  for (uint64_t i = 0; i < manager->n_actors; ++i) {
    // update_actor(manager->actors[i]);
  }
  return true;
}

entity actor_manager_get_by_id(actor_manager *manager, uint32_t id) {
  return manager->actors[id];
}
