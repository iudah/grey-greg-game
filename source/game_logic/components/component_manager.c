#include "component_manager.h"

#include "aabb_component.h"
#include "ai_component.h"
#include "position_component.h"
#include "render_component.h"
#include "rotation_component.h"
#include "scale_component.h"
#include "velocity_component.h"
#include "waypoint_component.h"
#include <stdio.h>

bool initialize_all_components() {
  printf("Initing stuff...");
  return initialize_position_component() && initialize_velocity_component() &&
         initialize_aabb_component() && initialize_waypoint_component() &&
         initialize_ai_component() && initialize_rotation_component() &&
         initialize_scale_component() && initialize_render_component();
}

void cleanup_all_components() {
  // ToDo: Add cleanup functions for each component
}

void static __attribute__((constructor(203))) init() {
  initialize_all_components();
}