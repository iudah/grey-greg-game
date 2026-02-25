#include <math.h>
#include <physics_system.h>
#include <string.h>
#include <zot.h>

#include "ai_component.h"
#include "component.h"
#include "simd.h"
#include "systems_manager.h"

#define FLEE_SPEED   0.5f
#define CHASE_SPEED  0.45f
#define PATROL_SPEED 0.4f

#define CHASE_RADIUS      5.0f
#define CHASE_LOST_RADIUS 7.5f
#define FLEE_RADIUS       2.0f
#define ATTACK_RANGE      1.0f

#define WAYPOINT_THRESHOLD GREY_ZERO

float distance(struct vec4_st *npc_pos, struct vec4_st *player_pos, bool flee,
               float32x4_t *diff_ptr);

void flee(entity e, struct vec4_st *player_pos, float vel[4]) {
  float32x4_t diff;

  float dist = distance(get_position(e), player_pos, true, &diff);

  if (dist < GREY_ZERO)
    // entity continues with it's velocity
    return;  // avoid div-by-zero

  auto velocity =
      vmulq_n_f32(diff, FLEE_SPEED / dist);  // normalize(npc_pos - player_pos)*FLEE_SPEED;

  vst1q_f32(vel, velocity);
}

void chase(entity e, struct vec4_st *player_pos, float vel[4]) {
  float32x4_t diff;

  float dist = distance(get_position(e), player_pos, false, &diff);

  if (dist < GREY_ZERO)
    // entity continues with it's velocity
    return;  // avoid div-by-zero

  auto velocity = vmulq_n_f32(diff, CHASE_SPEED / dist);  // normalize(npc_pos - player_pos);

  vst1q_f32(vel, velocity);
}

void perform_attack(entity e, entity player) {
  // TODO: implement
  // check attack point of e
}

void fight(entity e, entity player, float vel[4]) {
  if (distance(get_position(e), get_position(player), true, NULL) < ATTACK_RANGE) {
    perform_attack(e, player);
    memset(vel, 0, sizeof(*vel) * 4);
  } else {
    chase(e, get_position(player), vel);
  }
}

void patrol(entity e, float vel[4]) {
  float32x4_t diff;
  float dist = distance(get_position(e), (struct vec4_st *)get_next_patrol_point(e), false, &diff);

  if (dist < WAYPOINT_THRESHOLD) {
    advance_patrol_index(e);
  }

  if (dist < GREY_ZERO)
    // entity continues with it's velocity
    return;  // avoid div-by-zero

  auto velocity = vmulq_n_f32(diff, PATROL_SPEED / dist);  // normalize(npc_pos - player_pos);

  vst1q_f32(vel, velocity);
}

void idle(entity e) { set_velocity(e, (float[3]){0, 0, 0}); }

void ai_update_state(entity npc, entity player) {
  struct vec4_st *npc_pos = get_position(npc);
  struct vec4_st *player_pos = get_position(player);

  if (!npc_pos || !player_pos) return;

  float dist = distance(npc_pos, player_pos, false, NULL);
  ai_state *current = get_ai_state(npc);

  switch (*current) {
    case AI_PATROL:
      if (dist - CHASE_RADIUS < GREY_ZERO) {
        LOG("NPC %d transitioning PATROL → CHASE", npc.id);
        *current = AI_CHASE;
      }
      break;

    case AI_CHASE:
      if (dist - ATTACK_RANGE < GREY_ZERO) {
        LOG("NPC %d transitioning CHASE → ATTACK", npc.id);
        *current = AI_ATTACK;
      } else if (dist - CHASE_LOST_RADIUS > GREY_ZERO) {
        LOG("NPC %d transitioning CHASE → PATROL", npc.id);
        *current = AI_PATROL;
      }
      break;

    case AI_ATTACK:
      if (dist - ATTACK_RANGE > GREY_ZERO) {
        LOG("NPC %d transitioning ATTACK → CHASE", npc.id);
        *current = AI_CHASE;
      }
      break;

    case AI_FLEE:
      if (dist - FLEE_RADIUS > GREY_ZERO) {
        LOG("NPC %d transitioning FLEE → IDLE", npc.id);
        *current = AI_IDLE;
      }
      break;

    case AI_IDLE:
      if (dist - CHASE_RADIUS < GREY_ZERO) {
        LOG("NPC %d transitioning IDLE → CHASE", npc.id);
        *current = AI_CHASE;
      }
      break;
  }
}

void ai_system_update(game_logic *logic, float delta_time) {
  float velocity[4];
  extern entity player;

  for (uint32_t i = 0; i < ai_component->set.count; ++i) {
    entity npc = ai_component->set.dense[i];

    if (npc.generation == player.generation && npc.id == player.id) continue;
    ai_update_state(npc, player);

    ai_state *current = get_ai_state(npc);

    switch (*current) {
      case AI_IDLE:
        LOG("NPC %d is idle.", npc.id);
        idle(npc);
        break;

      case AI_PATROL:
        LOG("NPC %d is patrolling.", npc.id);
        patrol(npc, velocity);
        set_velocity(npc, velocity);
        break;

      case AI_CHASE:
        LOG("NPC %d is chasing the player!", npc.id);
        chase(npc, get_position(player), velocity);
        set_velocity(npc, velocity);
        break;

      case AI_ATTACK:
        LOG("NPC %d is attacking!", npc.id);
        fight(npc, player, velocity);
        set_velocity(npc, velocity);
        break;

      case AI_FLEE:
        LOG("NPC %d is fleeing!", npc.id);
        flee(npc, get_position(player), velocity);
        set_velocity(npc, velocity);
        break;
    }
  }
}

void static __attribute__((constructor(202))) initai_system_tl() {
  initialize_ai_component();
  register_system_update((system_update_fn_t)ai_system_update);
}
