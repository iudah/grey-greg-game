#ifndef EVENT_SYSTEM_H
#define EVENT_SYSTEM_H

#include "entity.h"

#define MAX_EVENT 63555

typedef enum {
  NO_EVENT,
  COLLISION_EVENT,
  KEY_DOWN_EVENT,
  KEY_RELEASED_EVENT,
  SCREEN_SIZE_CHANGED_EVENT,
  SCREEN_ORIENTATION_CHANGED_EVENT,
  NUMBER_OF_EVENT_TYPE
} event_type;

typedef struct {
  void *info;
  event_type type;
} event;

typedef struct event_system event_system;
typedef bool (*event_handler)(event *);

event_system *event_system_create(uint32_t initial_no_of_system);
void event_system_destroy(event_system *system);
void event_system_update(event_system *event_system);
bool event_trigger(event_system *event_system, void *info, const event_type type);
void event_handler_register(event_system *system, event_handler handle);

#endif