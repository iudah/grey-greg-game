#ifndef AI_COMPONENT_H
#define AI_COMPONENT_H

#include "component_base.h"
#include <grey_constants.h>


COMPONENT_DEFINE(ai);

bool initialize_ai_component() ;
ai_state *get_ai_state(entity e);
#endif
