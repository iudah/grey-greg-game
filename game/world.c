#include <game_logic/actor.h>
#include <math.h>

typedef struct generic_component generic_component_t;

entity person(float pos_x, float pos_y, float vel_x, float vel_y) {
  entity e = create_entity();

  actor_add_component(e, (generic_component_t *)position_component);
  actor_add_component(e, (generic_component_t *)velocity_component);
  actor_add_component(e, (generic_component_t *)aabb_component);
  actor_add_component(e, (generic_component_t *)waypoint_component);

  set_entity_position(e, pos_x, pos_y, 0);
  set_entity_velocity(e, vel_x, vel_y, 0);
  set_entity_aabb_lim(e, 3, 5, 1);

  return e;
}

entity rock(float pos_x, float pos_y) {
  entity e = create_entity();

  actor_add_component(e, (generic_component_t *)position_component);
  actor_add_component(e, (generic_component_t *)aabb_component);

  set_entity_position(e, pos_x, pos_y, 0);
  set_entity_aabb_lim(e, 3, 5, 0);

  return e;
}

void init_world() {
  entity rock_a = rock(0, 0);
  entity person_a = person(-10, 0, 1, 0);
  entity person_b = person(10, 0, -1, 0);
  entity person_c = person(15, 0, -3, 0);
}