#include "component.h"
#include <zot.h>
typedef enum {
  AI_IDLE,
  AI_PATROL,
  AI_CHASE,
  AI_ATTACK,
  AI_FLEE
} ai_state; // NPC state

struct ai_component {
  component_set set;
  ai_state *state;
};

struct ai_component *ai_component;

bool initialize_ai_component() {
  ai_component = zcalloc(1, sizeof(struct ai_component));
  return ai_component != NULL &&
         initialize_component((struct generic_component *)ai_component,
                              sizeof(ai_state));
}

void static __attribute__((__constructor__(200))) init() {
  initialize_ai_component();
}

ai_state get_ai_state(entity npc) { return ai_component->state[npc.id]; }

void ai_system_update() {
  ai_state *states = ai_component->state;

  for (uint32_t i = 0; i < ai_component->set.count; i++) {
    ai_state state = states[i];

    switch (state) {
    case AI_IDLE:
      LOG("NPC %d is idle.", ai_component->set.dense[i]);
      break;

    case AI_PATROL:
      LOG("NPC %d is patrolling.", ai_component->set.dense[i]);
      // patrol(npc);
      break;

    case AI_CHASE:
      LOG("NPC %d is chasing the player!", ai_component->set.dense[i]);
      // chase(npc);
      break;

    case AI_ATTACK:
      LOG("NPC %d is attacking!", ai_component->set.dense[i]);
      // attack(npc);
      break;

    case AI_FLEE:
      LOG("NPC %d is fleeing!", ai_component->set.dense[i]);
      // flee(npc);
      break;
    }
  }
}