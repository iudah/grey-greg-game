#ifndef EVENT_SYSTEM_H
#define EVENT_SYSTEM_H

#include "entity.h"

#warning Add new event types to event_type enum or use some int value

#define MAX_EVENT 63555

typedef enum {
  NO_EVENT,
  COLLISION_EVENT,
  KEY_DOWN_EVENT,
  KEY_RELEASED_EVENT,
  SCREEN_SIZE_CHANGED_EVENT,
  SCREEN_ORIENTATION_CHANGED_EVENT
} event_type;

typedef struct {
  void *info;
  event_type type;
} event;

typedef struct event_system event_system;
typedef struct event_queue event_queue;
typedef bool (*event_handler)(event *);

extern event_queue *event__queue;
extern event_system *event__system;

void event_default_broadcast();
event_system *get_default_event_default();
void event_handler_broadcast(event_system *system, event_queue *queue);
bool event_trigger(event_queue *q, void *info, int type);
void event_handler_register(event_system *system, event_handler handle);

#endif