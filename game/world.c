#include <aabb_component.h>
#include <actor.h>
#include <force_component.h>
#include <game_main.h>
#include <gravity_system.h>
#include <inttypes.h>
#include <mass_component.h>
#include <math.h>
#include <physics_system.h>
#include <position_component.h>
#include <raylib_glue.h>
#include <render_component.h>
#include <render_system.h>
#include <stdint.h>
#include <systems_manager.h>
#include <velocity_component.h>
#include <waypoint_component.h>
#include <zot.h>

typedef struct generic_component generic_component_t;

#define SPRITE_X 20
#define SPRITE_Y 20

entity sprite(float pos_x, float pos_y, uint32_t rgba) {
  entity e = create_entity();

  actor_add_component(e, (generic_component_t *)position_component);
  actor_add_component(e, (generic_component_t *)aabb_component);
  actor_add_component(e, (generic_component_t *)render_component);

  set_entity_position(e, pos_x, pos_y, 0);
  set_entity_aabb_lim(e, SPRITE_X, SPRITE_Y, 0);

  set_entity_color(e, rgba);

  return e;
}

entity terrain(float pos_x, float pos_y, uint32_t rgba) { return sprite(pos_x, pos_y, rgba); }

entity person(float pos_x, float pos_y, float vel_x, float vel_y) {
  entity e = sprite(pos_x, pos_y, 0xb5651d);
  actor_add_component(e, (generic_component_t *)velocity_component);
  actor_add_component(e, (generic_component_t *)force_component);
  actor_add_component(e, (generic_component_t *)mass_component);
  // set_entity_velocity(e, vel_x, vel_y, 0);
  set_entity_velocity(e, 0, 0, 0);
  set_entity_mass(e, 10);

  return e;
}

entity rock(float pos_x, float pos_y) { return terrain(pos_x, pos_y, 0x7f8386); }

entity grass(float pos_x, float pos_y) { return terrain(pos_x, pos_y, 0x3F9B0B); }

entity mud(float pos_x, float pos_y) { return terrain(pos_x, pos_y, 0xb5651d); }

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

bool player_movement(event *e) {
#ifndef NO_RAYLIB
#define XY_ACCEL 10.f
  if (e->type == KEY_DOWN_EVENT) {
    switch (*(KeyboardKey *)e->info) {
      case KEY_UP:
        add_force(player, (float[]){0, -110 * get_mass(player), 0, 0});
        break;

      case KEY_LEFT:
        add_force(player, (float[]){-XY_ACCEL * get_mass(player), 0, 0, 0});
        break;

      case KEY_RIGHT:
        add_force(player, (float[]){XY_ACCEL * get_mass(player), 0, 0, 0});
        break;

      default:
        break;
    }
  }

  if (e->type == KEY_RELEASED_EVENT) {
    // LOG("%s\n", __FUNCTION__);
    // exit(0);
    switch (*(int *)e->info) {
      case KEY_UP:
        // add_force(player, (float[]){0, 11 * get_mass(player), 0, 0});
        break;

      case KEY_LEFT:
      case KEY_RIGHT:
        add_force(player,
                  (float[]){-get_velocity(player)->x / TIMESTEP / FORCE_SCALE * get_mass(player), 0,
                            0, 0});
        break;

      default:
        break;
    }
  }
#endif
  return true;
}

void init_world() {
  LOG("%s", __FUNCTION__);
  set_game_screen_config(SCREEN_X, SCREEN_Y, "Grey Greg", 60);

  register_system_update((system_update_fn_t)gravity_system_update);
  register_system_update((system_update_fn_t)physics_system_update);
  register_system_update((system_update_fn_t)clear_forces);
  register_system_update((system_update_fn_t)render_system_update);

  event_handler_register(get_default_event_default(), walk_through_resolution);
  event_handler_register(event__system, player_movement);

#define EPSILON  (GREY_AABB_GAP)
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
    bool is_mixed;  // randomizes texture
  } levels[] = {
      // --- TIER 1 ---
      {grass, 80, 100, 2, 0, false},

      // --- TIER 2 ---
      {mud, 500, 220, 5, 1, false},

      // --- TIER 3 ---
      {rock, 100, 340, 4, 1, false},   // Left Island
      {grass, 380, 340, 5, 1, false},  // Center Island

      // --- TIER 4 ---
      {mud, 600, 480, 6, 1, true},  // Mixed texture

      // --- TIER 5 ---
      {grass, 60, 600, 8, 2, true},  // Left long segment
      {rock, 550, 600, 7, 1, true}   // Right segment
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
          world_append_sprite(person(current_x, current_y - 40 - EPSILON * 4, 10, 0));
        }
      }
    }
  }

  // Player
  float p_x = levels[0].x + EPSILON * 4;
  float p_y = levels[0].y - 40 - EPSILON * 4;

  // A slave's dream is not of freedom but of a slave to call his own

  world_append_sprite(player = person(p_x, p_y, 10, 0));
}