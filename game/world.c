#include <game_logic/actor.h>

typedef struct generic_component generic_component_t;

void init_world() {
  entity actors[10];
  for (int i = 0; i < 10; ++i) {
    actors[i] = create_actor();

    actor_add_component(actors[i], (generic_component_t *)position_component);
    actor_add_component(actors[i], (generic_component_t *)velocity_component);
    actor_add_component(actors[i], (generic_component_t *)aabb_component);
    actor_add_component(actors[i], (generic_component_t *)waypoint_component);
  }
}