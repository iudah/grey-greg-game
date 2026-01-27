#ifndef PROCESS_MANAGER_H
#define PROCESS_MANAGER_H

typedef struct process_manager process_manager;
process_manager *process_manager_create();
void process_manager_destroy(process_manager *mgr);
void process_manager_update(process_manager *mgr, float delta_time);

#endif