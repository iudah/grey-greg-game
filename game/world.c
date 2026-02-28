#include <actor.h>
#include <collision_component.h>
#include <force_component.h>
#include <game_logic.h>
#include <game_main.h>
#include <gravity_system.h>
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
  // entity e = sprite(pos_x, pos_y, 0xb5651d);
  entity e = sprite(pos_x, pos_y, 0);
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

  if (e->type == KEY_DOWN_EVENT) {
    switch (*(KeyboardKey *)e->info) {
      case KEY_UP: {
        add_force(player, (float[]){0, -JUMP * *mass, 0, 0});
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
    // LOG("%s\n", __FUNCTION__);
    // exit(0);
    switch (*(int *)e->info) {
      case KEY_UP:
        // add_force(player, (float[]){0, 11 * get_mass(player), 0, 0});
        break;

      case KEY_LEFT:
      case KEY_RIGHT:
        add_force(player, (float[]){-get_velocity(player)->x / TIMESTEP / *mass, 0, 0, 0});
        break;

      default:
        break;
    }
  }
#endif
  return true;
}

void init_world(game_logic *logic) {
  LOG("%s", __FUNCTION__);

  register_system_update((system_update_fn_t)gravity_system_update);
  register_system_update((system_update_fn_t)physics_system_update);
  register_system_update((system_update_fn_t)clear_forces);

  event_handler_register(game_logic_get_event_system(logic), walk_through_resolution);
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

  uint32_t air_tile = A = resource_make_tile(resc_mgr, 0, 0, 0, 0, 0, TILE_WALKABLE);
  uint32_t grass_tile = G =
      resource_make_tile(resc_mgr, world_tile_set, 0, 0, tile_size, tile_size, TILE_SOLID);
  uint32_t dirt_tile = D = resource_make_tile(resc_mgr, world_tile_set, 0, 1 * tile_size, tile_size,
                                              tile_size, TILE_SOLID);
  uint32_t ladder_tile = L = resource_make_tile(resc_mgr, world_tile_set, 9 * tile_size,
                                                3 * tile_size, tile_size, tile_size, TILE_SOLID);

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
  player = person(start_pt[0] * 16 + 8 + 4, start_pt[1] * 16 + 8 - 4, 0, 0);
}