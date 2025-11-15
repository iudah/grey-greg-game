#include "event_system.h"

#include <stdlib.h>
#include <zot.h> //a custom memory manager

#ifdef _WIN32
#include <windows.h>
#else
#include <time.h>
#endif

#ifdef _WIN32
#define signal(...)
typedef LARGE_INTEGER ztimespec;
double freq;
static inline void get_win32_frequency();
#else
typedef struct timespec ztimespec;
#define get_win32_frequency(...)
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

typedef struct {
  void *info;
  event_type type;
} event;

typedef bool (*event_handler)(event *);

struct event_system {
  event_handler *event_handler; // list of handlers
  int no_event_handler;         // number of handlers
  int avail_no_event_handler;   // number of available handler
};

// cyclic queue
struct event_queue {
  event **queue;
  int head;
  int tail;
  int no_event;
};

bool event_enqueue(event_queue *q, event *e);

void *create_event_system() { return zcalloc(1, sizeof(event_system)); }

void initialize_event_system(event_system *system,
                             uint32_t initial_no_of_system) {
  if (!initial_no_of_system)
    initial_no_of_system = 1;
  system->event_handler =
      zcalloc(initial_no_of_system, sizeof(*system->event_handler));
  system->no_event_handler = 0;
  system->avail_no_event_handler = initial_no_of_system;
}

void initialize_event_queue(event_queue *queue, uint32_t initial_no_of_event) {
  if (!initial_no_of_event)
    initial_no_of_event = 100;
  queue->no_event = initial_no_of_event;
  queue->queue = zcalloc(initial_no_of_event, sizeof(*queue->queue));
}

event_queue _queue_;
event_queue *event__queue = &_queue_;

event_system _sys_;
event_system *event__system = &_sys_;

void __attribute__((constructor)) init() {

  initialize_event_queue(event__queue, MAX_EVENT);
  initialize_event_system(event__system, MAX_EVENT);
}

// broadcast may run on a seperate thread
void event_handler_broadcast(event_system *system, event_queue *queue) {
  for (; queue->head != queue->tail; ++queue->head) {

    event *event = queue->queue[queue->head];

    for (int i = 0; i < system->no_event_handler; ++i) {
      if (event->type) {
        system->event_handler[i](event);
      }
    }

    if (queue->head == queue->no_event)
      queue->head = 0;
  }
}

void event_handler_register(event_system *system, event_handler handle) {
  if (!handle) {
    LOG("Event handler is null.");
    return;
  }
  if (system->no_event_handler >= MAX_EVENT_HANDLER) {
    LOG_ERROR("Maximum number of event handler reached for this broadcaster.");
    return;
  }

  system->event_handler[system->no_event_handler] = handle;
  system->no_event_handler++;
}

// triggers may occur on multiple threads
bool event_trigger(event_queue *q, void *info, int type) {
  event *e = zmalloc(sizeof((*e)));

  e->info = info;
  e->type = type;

  return event_enqueue(q, e);
}

bool event_enqueue(event_queue *q, event *e) {
  if (q->tail == q->no_event)
    q->tail = 0;

  if (q->tail == q->head)
    return false;

  q->queue[q->tail] = e;
  ++q->tail;

  return true;
}
