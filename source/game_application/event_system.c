#include "event_system.h"

#include <inttypes.h>
#include <stdlib.h>
#include <zot.h>  //a custom memory manager
#include <grey_constants.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <time.h>
#endif

#ifndef DEBUG
#ifdef LOG
#undef LOG
#endif
#define LOG(...)

#ifdef LOG_ERROR
#undef LOG_ERROR
#endif
#define LOG_ERROR(...)
#endif

#define MAX_EVENT_HANDLER 25

// cyclic queue
struct event_queue {
  event **queue;
  size_t head;
  size_t tail;
  size_t capacity;
};

struct event_system {
  struct event_queue queues[2];
  event_handler *event_handler;  // list of handlers
  int no_event_handler;          // number of handlers
  int avail_no_event_handler;    // number of available handler
  char active_idx;
};

typedef struct event_queue event_queue;

void initialize_event_queue(event_queue *queue, uint32_t initial_no_of_event) {
  if (!initial_no_of_event) initial_no_of_event = 100;
  queue->capacity = initial_no_of_event;
  queue->head = queue->tail = 0;
  queue->queue = zcalloc(initial_no_of_event, sizeof(*queue->queue));
}

void deinitialize_event_queue(event_queue *queue) { zfree(queue->queue); }

event_system *event_system_create(uint32_t initial_no_of_system) {
  event_system *system = zmalloc(sizeof(*system));
  if (!initial_no_of_system) initial_no_of_system = MAX_EVENT_HANDLER;
  system->event_handler = zcalloc(initial_no_of_system, sizeof(*system->event_handler));
  system->no_event_handler = 0;
  system->avail_no_event_handler = initial_no_of_system;

  initialize_event_queue(&system->queues[0], 0);
  initialize_event_queue(&system->queues[1], 0);

  return system;
}

void event_system_destroy(event_system *system) {
  deinitialize_event_queue(&system->queues[0]);
  deinitialize_event_queue(&system->queues[1]);
  zfree(system->event_handler);
  zfree(system);
}

static inline bool is_empty(event_queue *q) { return q->head == q->tail; }

static inline bool is_full(event_queue *q) { return (q->tail + 1) % q->capacity == q->head; }
// broadcast may run on a seperate thread
void event_handler_broadcast(event_system *system, event_queue *queue) {
  // LOG("Broadcasting events.");

  uint32_t u = 0;

  for (; queue->head != queue->tail; queue->head = (queue->head + 1) % queue->capacity) {
    event *event = queue->queue[queue->head];

    // let zot handle cleaning automatically
    queue->queue[queue->head] = 0;

    for (int i = 0; i < system->no_event_handler; ++i) {
      if (system->event_handler[i]) {
        system->event_handler[i](event);
      }
    }
    if (event->info) {
      // todo: add destructors?
    }
    zfree(event);
  }
}

void event_handler_register(event_system *system, event_handler handle) {
  if (!handle) {
    LOG("Event handler is null.");
    return;
  }
  if (system->no_event_handler >= MAX_EVENT_HANDLER) {
    // todo: reallocate
    LOG_ERROR("Maximum number of event handler reached for this broadcaster.");
    return;
  }

  system->event_handler[system->no_event_handler] = handle;
  system->no_event_handler++;
}

bool event_enqueue(event_queue *q, event *e) {
  if (is_full(q)) return false;

  q->queue[q->tail] = e;
  q->tail = (q->tail + 1) % q->capacity;

  return true;
}

// triggers may occur on multiple threads
bool event_trigger(event_system *event_system, void *info, const event_type type) {
  event *e = zmalloc(sizeof((*e)));

  e->info = info;
  e->type = type;

  return event_enqueue(&event_system->queues[event_system->active_idx], e);
}

void event_system_update(event_system *event_system) {
  char active_idx = event_system->active_idx;

  event_system->active_idx ^= 1;

  event_queue *queue_to_process = &event_system->queues[active_idx];

  event_handler_broadcast(event_system, queue_to_process);
}
