#ifndef EVENT_SYSTEM_H
#define EVENT_SYSTEM_H

#include "entity.h"

#warning Add new event types to event_type enum or use some int value

#define MAX_EVENT 63555

typedef enum
{
    NO_EVENT,
    COLLISION_EVENT
} event_type;

typedef struct event_system event_system;
typedef struct event_queue event_queue;

extern event_queue *event__queue;
extern event_system *event__system;

void event_default_broadcast();
void event_handler_broadcast(event_system *system, event_queue *queue);
bool event_trigger(event_queue *q, void *info, int type);
void event_enqueue_collision(entity entity_i, entity entity_j);

#endif