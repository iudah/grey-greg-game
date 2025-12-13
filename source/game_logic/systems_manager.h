#ifndef SYSTEMS_MANAGER_H
#define SYSTEMS_MANAGER_H

typedef void (*system_update_fn_t)(void);

bool register_system(system_update_fn_t system);

#endif