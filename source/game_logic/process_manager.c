#include "process_manager.h"

#include <zot.h>

struct process_manager {};

process_manager *process_manager_create() {
  process_manager *mgr = zmalloc(sizeof(*mgr));
  return mgr;
}

void process_manager_destroy(process_manager *mgr) { zfree(mgr); }

void process_manager_update(process_manager *mgr, float delta_time) {}
