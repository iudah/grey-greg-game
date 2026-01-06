#include "actor.h"

entity create_actor() {
  entity actor = create_entity();
  return actor;
}

bool actor_add_component(entity actor, struct generic_component* component) {
  return attach_component(actor, component);
}

void actor_remove_component(entity actor, struct generic_component* component) {
  return detach_component(actor, component);
}

void destroy_actor(entity actor) { return destroy_entity(actor); }