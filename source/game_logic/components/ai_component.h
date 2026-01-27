#ifndef AI_COMPONENT_COMPONENT_H
#define AI_COMPONENT_COMPONENT_H

#include "component_base.h"

typedef enum ai_state_enum ai_state;
enum ai_state_enum { AI_IDLE, AI_PATROL, AI_CHASE, AI_ATTACK, AI_FLEE };

struct ai_component {
  component_set set;
  struct {
    ai_state* state;
  }* streams;
};

extern struct ai_component* ai_component;

bool initialize_ai_component();

#endif
