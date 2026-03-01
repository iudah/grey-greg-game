#include <actor.h>
#include <collision_component.h>
#include <force_component.h>
#include <game_logic.h>
#include <game_main.h>
#include <gravity_system.h>
#include <grey_constants.h>
#include <grid.h>
#include <grid_component.h>
#include <inttypes.h>
#include <mass_component.h>
#include <math.h>
#include <physics_system.h>
#include <position_component.h>
#include <raylib.h>
#include <raylib_glue.h>
#include <render_component.h>
#include <render_system.h>
#include <resource_manager.h>
#include <stdint.h>
#include <systems_manager.h>
#include <velocity_component.h>
#include <wave_fn_collapse.h>
#include <waypoint_component.h>
#include <zot.h>

typedef struct generic_component generic_component_t;

#define LAYER_PLAYER  COLLISION_LAYER(0)
#define LAYER_TERRAIN COLLISION_LAYER(1)
#define LAYER_ENEMY   COLLISION_LAYER(2)
#define LAYER_PICKUP  COLLISION_LAYER(3)
#define LAYER_HAZARD  COLLISION_LAYER(4)

#define XY_ACCEL         (40.f)
#define JUMP_SPEED       (-2.5f)  // Tiles per Frame
#define MAX_JUMP         (2)
#define JUMP_HOLD_FRAMES (10)

struct jump_state {
  uint32_t jumps;
  uint32_t jump_timer;
  bool can_jump;
  bool is_in_air;
  bool key_held;
  bool prev_key_held;
  bool on_ground;
};

struct jump_state player_jump_state = {0, 0, true, false, false, false, false};

entity sprite(float pos_x, float pos_y, uint32_t rgba) {
  entity e = create_entity();

  actor_add_component(e, (generic_component_t *)position_component);
  actor_add_component(e, (generic_component_t *)collision_component);
  actor_add_component(e, (generic_component_t *)render_component);

  set_entity_position(e, pos_x, pos_y, 0);
  set_entity_collision_extent(e, SPRITE_X, SPRITE_Y, 0);

  set_entity_color(e, rgba);

  return e;
}

entity terrain(float pos_x, float pos_y, uint32_t rgba) { return sprite(pos_x, pos_y, rgba); }

entity person(float pos_x, float pos_y, float vel_x, float vel_y) {
  entity e = sprite(pos_x, pos_y, 0xb5651d << 8 | 100);
  actor_add_component(e, (generic_component_t *)velocity_component);
  actor_add_component(e, (generic_component_t *)force_component);
  actor_add_component(e, (generic_component_t *)mass_component);
  // set_entity_velocity(e, vel_x, vel_y, 0);
  set_entity_velocity(e, 0, 0, 0);
  set_entity_mass(e, 10);
  set_entity_collision_flag(e, COLLISION_SOLID_2D);
  set_entity_collision_layer(e, LAYER_ENEMY);
  set_entity_collision_mask(e, LAYER_TERRAIN);
  return e;
}

entity make_player(float pos_x, float pos_y) {
  entity e = person(pos_x, pos_y, 0, 0);
  set_entity_collision_layer(e, LAYER_PLAYER);
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

bool game_append_sprite(entity e) {
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
  float *mass = get_mass(player);
  if (!mass) return false;

  if (e->type == COLLISION_EVENT) {
    collision_data *collision = (collision_data *)e->info;
    entity other;
    if (is_same_entity(collision->a, player)) {
      other = collision->b;
    } else if (is_same_entity(collision->b, player)) {
      other = collision->a;
    } else {
      return false;
    }

    struct vec4_st *player_pos = get_position(player);
    struct vec4_st *other_pos = get_position(other);
    if (!other_pos) return false;

    if (player_pos->y > other_pos->y && (*get_collision_flag(other) & COLLISION_FACE_UP)) {
      player_jump_state.on_ground = true;
    }
  }

  if (e->type == KEY_DOWN_EVENT) {
    switch (*(KeyboardKey *)e->info) {
      case KEY_UP: {
        player_jump_state.key_held = true;
        DrawText("UP", 160, 160, 240, RED);
        LOG("UP");
      } break;

      case KEY_LEFT:
        add_force(player, (float[]){-XY_ACCEL * *mass, 0, 0, 0});
        break;

      case KEY_RIGHT:
        add_force(player, (float[]){XY_ACCEL * *mass, 0, 0, 0});
        break;

      default:
        break;
    }
  }

  if (e->type == KEY_RELEASED_EVENT) {
    switch (*(int *)e->info) {
      case KEY_UP: {
        player_jump_state.key_held = false;
      } break;

      case KEY_LEFT:
      case KEY_RIGHT:
        add_force(player, (float[]){-get_velocity(player)->x * *mass / (TIMESTEP * 0.3f), 0, 0, 0});
        break;

      default:
        break;
    }
  }
#endif
  return true;
}

bool player_update(event *e) {
  if (player_jump_state.on_ground) {
    player_jump_state.jumps = 0;
  } else {
    if (!player_jump_state.jumps) {
      player_jump_state.jumps = 1;
    }
  }

  if (player_jump_state.key_held && !player_jump_state.prev_key_held &&
      player_jump_state.jumps < MAX_JUMP) {
    player_jump_state.jumps++;
    player_jump_state.jump_timer = JUMP_HOLD_FRAMES;
    player_jump_state.on_ground = false;
  }

  if (!player_jump_state.key_held) {
    player_jump_state.jump_timer = 0;
  }

  if (player_jump_state.jump_timer > 0) {
    float *mass = get_mass(player);
    if (!mass) return false;

    add_force(player, (float[]){0, JUMP_SPEED * *mass, 0, 0});
    player_jump_state.jump_timer--;
    player_jump_state.on_ground = false;
  }

  player_jump_state.prev_key_held = player_jump_state.key_held;
  player_jump_state.on_ground = false;  // reset each frame for collision event to set it back

  return true;
}

void collision_resolve_cb(entity a, entity b, int axis, collision_flag direction) {
  if (axis != 1) return;

  bool landing_on_something = direction == COLLISION_FACE_DOWN;
  bool lifted_by_something = direction == COLLISION_FACE_UP;

  if ((is_same_entity(a, player) && landing_on_something) ||
      (is_same_entity(b, player) && lifted_by_something)) {
    player_jump_state.on_ground = true;
  }
}

void init_world(game_logic *logic) {
  LOG("%s", __FUNCTION__);

  register_system_update((system_update_fn_t)gravity_system_update);
  register_system_update((system_update_fn_t)physics_system_update);
  register_system_update((system_update_fn_t)player_update);
  register_system_update((system_update_fn_t)clear_forces);

  register_collision_resolve_callback(collision_resolve_cb);

  event_handler_register(game_logic_get_event_system(logic), player_movement);

  // A World (map) has multiple biomes (sub-map).
  entity world_entity = create_entity();
  game_append_sprite(world_entity);
  attach_component(world_entity, (struct generic_component *)grid_component);
  attach_component(world_entity, (struct generic_component *)render_component);

  grid *game_map = grid_create(45, 24, 16, 16);
  grid_component_set_grid(world_entity, game_map);

  resource_manager *resc_mgr = game_logic_get_resource_manager(logic);

  const int tile_size = GREY_TILE_SIZE;
  uint32_t world_tile_set = resource_load_texture(resc_mgr, "world_tile_set.png");
  SetTextureFilter(resource_get_texture(resc_mgr, world_tile_set), TEXTURE_FILTER_POINT);

  uint32_t A, G, D, R, I, L;

  A = resource_make_tile(resc_mgr, 0, 0, 0, 0, 0, COLLISION_AIR, COLLISION_AIR, COLLISION_AIR);
  uint32_t air_tile = A;

  G = resource_make_tile(resc_mgr, world_tile_set, 0, 0, tile_size, tile_size, COLLISION_SOLID,
                         LAYER_TERRAIN, LAYER_PLAYER | LAYER_ENEMY);
  uint32_t grass_tile = G;

  D = resource_make_tile(resc_mgr, world_tile_set, 0, 1 * tile_size, tile_size, tile_size,
                         COLLISION_SOLID, LAYER_TERRAIN, LAYER_PLAYER | LAYER_ENEMY);
  uint32_t dirt_tile = D;

  L = resource_make_tile(resc_mgr, world_tile_set, 9 * tile_size, 3 * tile_size, tile_size,
                         tile_size, COLLISION_TRIGGER, LAYER_TERRAIN, LAYER_PLAYER);
  uint32_t ladder_tile = L;

  wfc_atlas *wfc_rules = wfc_atlas_create(4);

  wfc_atlas_add_rule(wfc_rules, air_tile, air_tile, air_tile, air_tile, air_tile);
  wfc_atlas_add_rule(wfc_rules, grass_tile, air_tile, air_tile, air_tile, air_tile);
  wfc_atlas_add_rule(wfc_rules, grass_tile, air_tile, grass_tile, dirt_tile, grass_tile);
  wfc_atlas_add_rule(wfc_rules, dirt_tile, grass_tile, dirt_tile, dirt_tile, dirt_tile);
  wfc_atlas_add_rule(wfc_rules, dirt_tile, dirt_tile, dirt_tile, dirt_tile, dirt_tile);
  wfc_atlas_add_rule(wfc_rules, ladder_tile, air_tile, dirt_tile, dirt_tile, dirt_tile);
  wfc_atlas_add_rule(wfc_rules, ladder_tile, air_tile, grass_tile, grass_tile, grass_tile);

  wfc_atlas_compile(wfc_rules);

  uint32_t start_pt[] = {2, 2};

  if (!wfc_run_collapse(game_map, wfc_rules, resc_mgr, start_pt)) {
    LOG_ERROR("Critical WFC failure.");
  }

  // bake map
  RenderTexture2D *game_map_cache = grid_bake(game_map, resc_mgr);
  grid_set_grid_cache(game_map, game_map_cache);

  // A slave's dream is not of freedom but of a slave to call his own
  player = make_player(start_pt[0] * 16 + 8, start_pt[1] * 16 + 8);
}