#ifndef COMPONENTS_MANAGER_H
#define COMPONENTS_MANAGER_H

#include "aabb_component.h"
#include "ai_component.h"
#include "position_component.h"
#include "rotation_component.h"
#include "velocity_component.h"
#include "waypoint_component.h"

bool initialize_all_components();
void cleanup_all_components();

#endif