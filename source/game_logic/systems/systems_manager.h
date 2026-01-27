#ifndef SYSTEMS_MANAGER_H
#define SYSTEMS_MANAGER_H
#include <game_logic.h>

typedef void (*system_update_fn_t)(game_logic *logic, float delta_time);

bool register_system_update(system_update_fn_t system);
#endif