#ifndef AI_COMPONENT_H
#define AI_COMPONENT_H

#include "component_base.h"

typedef enum ai_state_enum ai_state;
enum ai_state_enum { AI_IDLE, AI_PATROL, AI_CHASE, AI_ATTACK, AI_FLEE };

COMPONENT_DEFINE(ai);

bool initialize_ai_component() ;
ai_state *get_ai_state(entity e);
#endif
