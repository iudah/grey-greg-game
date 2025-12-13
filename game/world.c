#include <aabb_component.h>
#include <actor.h>
#include <math.h>
#include <position_component.h>
#include <render_component.h>
#include <velocity_component.h>
#include <waypoint_component.h>

typedef struct generic_component generic_component_t;

entity person(float pos_x, float pos_y, float vel_x, float vel_y) {
  entity e = create_entity();

  actor_add_component(e, (generic_component_t *)position_component);
  actor_add_component(e, (generic_component_t *)velocity_component);
  actor_add_component(e, (generic_component_t *)aabb_component);
  actor_add_component(e, (generic_component_t *)waypoint_component);
  actor_add_component(e, (generic_component_t *)render_component);

  set_entity_position(e, pos_x, pos_y, 0);
  set_entity_velocity(e, vel_x, vel_y, 0);
  set_entity_aabb_lim(e, 30, 50, 0);

  set_entity_color(e, 0xabababff);

  return e;
}

entity rock(float pos_x, float pos_y) {
  entity e = create_entity();

  actor_add_component(e, (generic_component_t *)position_component);
  actor_add_component(e, (generic_component_t *)aabb_component);

  set_entity_position(e, pos_x, pos_y, 0);
  set_entity_aabb_lim(e, 30, 50, 0);

  return e;
}

void init_world() {
  // entity rock_a = rock(100, 100);
  entity person_a = person(100 - 60, 100, 10, 0);
  entity person_b = person(110, 100, -10, 0);
  entity person_c = person(200, 100, -30, 0);
}