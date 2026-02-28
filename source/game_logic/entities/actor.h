#ifndef ACTOR_H
#define ACTOR_H

#include "component.h"
#include "entity.h"


entity create_actor();
bool actor_add_component(entity actor, struct generic_component* component);
void actor_remove_component(entity actor, struct generic_component* component);
void destroy_actor(entity actor);
#endif
