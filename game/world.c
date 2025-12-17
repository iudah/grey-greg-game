#include <aabb_component.h>
#include <actor.h>
#include <inttypes.h>
#include <math.h>
#include <position_component.h>
#include <render_component.h>
#include <stdint.h>
#include <velocity_component.h>
#include <waypoint_component.h>
#include <zot.h>

typedef struct generic_component generic_component_t;

#define SPRITE_X 20
#define SPRITE_Y 20

entity character(float pos_x, float pos_y, float vel_x, float vel_y,
                 uint32_t rgba) {
  entity e = create_entity();

  actor_add_component(e, (generic_component_t *)position_component);
  actor_add_component(e, (generic_component_t *)velocity_component);
  actor_add_component(e, (generic_component_t *)aabb_component);
  // actor_add_component(e, (generic_component_t *)waypoint_component);
  actor_add_component(e, (generic_component_t *)render_component);

  set_entity_position(e, pos_x, pos_y, 0);
  set_entity_velocity(e, vel_x, vel_y, 0);
  set_entity_aabb_lim(e, SPRITE_X, SPRITE_Y, 0);

  set_entity_color(e, rgba);

  return e;
}

entity person(float pos_x, float pos_y, float vel_x, float vel_y) {
  return character(pos_x, pos_y, vel_x, vel_y, 0xb5651d);
}

entity rock(float pos_x, float pos_y) {
  return character(pos_x, pos_y, 0, 0, 0x7f8386);
}

entity grass(float pos_x, float pos_y) {
  return character(pos_x, pos_y, 0, 0, 0x3F9B0B);
}

entity mud(float pos_x, float pos_y) {
  return character(pos_x, pos_y, 0, 0, 0xb5651d);
}

struct {
  entity *sprite;
  uint32_t count;
  uint32_t cap;
} world;

bool world_append_sprite(entity e) {
  if (!world.cap) {
    world.cap = 32;
    world.sprite = zmalloc(world.cap * sizeof(*world.sprite));
  }

  if (world.cap == (world.count + 1)) {
    world.cap += 32;
    auto tmp = zrealloc(world.sprite, world.cap * sizeof(*world.sprite));
    if (!tmp) {
      return false;
    }
    world.sprite = tmp;
  }

  world.sprite[world.count++] = e;
  return true;
}
void init_world() {

#define EPSILON 0.0001
#define SPRITE_W (SPRITE_X + EPSILON)
#define SPRITE_H (SPRITE_Y + EPSILON)

  float step_x = SPRITE_W * 2;
  float step_y = SPRITE_H * 2;

  // Border
  for (float y = 0; (y - SPRITE_H) < SCREEN_Y; y += step_y) {
    for (float x = 0; (x - SPRITE_W) < SCREEN_X; x += step_x) {
      bool is_start = (x < SPRITE_W || y < SPRITE_H);
      bool is_end = (x + SPRITE_W > SCREEN_X || y + SPRITE_H > SCREEN_Y);

      if (is_start || is_end) {
        world_append_sprite(rock(x, y));
      }
    }
  }

  // Platform
  struct {
    entity (*base_type)(float, float);
    float x, y;
    uint32_t length;
    uint32_t enemies;
    bool is_mixed; // randomizes texture
  } levels[] = {
      // --- TIER 1 ---
      {grass, 80, 100, 2, 0, false},

      // --- TIER 2 ---
      {mud, 500, 220, 5, 1, false},

      // --- TIER 3 ---
      {rock, 100, 340, 4, 1, false},  // Left Island
      {grass, 380, 340, 5, 1, false}, // Center Island

      // --- TIER 4 ---
      {mud, 600, 480, 6, 1, true}, // Mixed texture

      // --- TIER 5 ---
      {grass, 60, 600, 8, 2, true}, // Left long segment
      {rock, 550, 600, 7, 1, true}  // Right segment
  };

  // Platforms & Enemies
  size_t num_platforms = sizeof(levels) / sizeof(levels[0]);

  for (uint32_t i = 0; i < num_platforms; ++i) {

    for (uint32_t j = 0; j < levels[i].length; ++j) {

      float current_x = levels[i].x + j * step_x;
      float current_y = levels[i].y;

      entity (*current_mat)(float, float) = levels[i].base_type;

      if (levels[i].is_mixed) {
        if (j % 3 == 0)
          current_mat = rock;
        else if (j % 2 == 0)
          current_mat = mud;
        else
          current_mat = grass;
      }

      world_append_sprite(current_mat(current_x, current_y));

      // Enemy spawning
      if (levels[i].enemies > 0) {
        if (j >= levels[i].length - levels[i].enemies) {
          world_append_sprite(person(current_x, current_y - 40, 10, 0));
        }
      }
    }
  }

  // Player
  float p_x = levels[0].x + EPSILON * 4;
  float p_y = levels[0].y - 40;

  // A slave's dream is not of freedom but of a slave to call his own

  world_append_sprite(person(p_x, p_y, 10, 0));
}