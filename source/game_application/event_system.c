#include "event_system.h"

#include <inttypes.h>
#include <stdlib.h>
#include <zot.h>  //a custom memory manager

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

struct event_system {
  event_handler* event_handler;  // list of handlers
  int no_event_handler;          // number of handlers
  int avail_no_event_handler;    // number of available handler
};

// cyclic queue
struct event_queue {
  event** queue;
  size_t head;
  size_t tail;
  size_t capacity;
};

bool event_enqueue(event_queue* q, event* e);
bool event_destroy(event* e);

void* create_event_system() { return zcalloc(1, sizeof(event_system)); }

void initialize_event_system(event_system* system,
                             uint32_t initial_no_of_system) {
  if (!initial_no_of_system) initial_no_of_system = 1;
  system->event_handler =
      zcalloc(initial_no_of_system, sizeof(*system->event_handler));
  system->no_event_handler = 0;
  system->avail_no_event_handler = initial_no_of_system;
}

void initialize_event_queue(event_queue* queue, uint32_t initial_no_of_event) {
  if (!initial_no_of_event) initial_no_of_event = 100;
  queue->capacity = initial_no_of_event;
  queue->head = queue->tail = 0;
  queue->queue = zcalloc(initial_no_of_event, sizeof(*queue->queue));
}

event_queue _queue_;
event_queue* event__queue = &_queue_;

event_system _sys_;
event_system* event__system = &_sys_;

void __attribute__((constructor)) init() {
  initialize_event_queue(event__queue, MAX_EVENT);
  initialize_event_system(event__system, MAX_EVENT);
}

void event_default_broadcast() {
  event_handler_broadcast(event__system, event__queue);
}

event_system* get_default_event_default() { return event__system; }

static inline bool is_empty(event_queue* q) { return q->head == q->tail; }

static inline bool is_full(event_queue* q) {
  return (q->tail + 1) % q->capacity == q->head;
}
// broadcast may run on a seperate thread
void event_handler_broadcast(event_system* system, event_queue* queue) {
  // LOG("Broadcasting events.");

  uint32_t u = 0;

  for (; queue->head != queue->tail;
       queue->head = (queue->head + 1) % queue->capacity) {
    event* event = queue->queue[queue->head];

    // let zot handle cleaning automatically
    queue->queue[queue->head] = 0;

    for (int i = 0; i < system->no_event_handler; ++i) {
      if (event->type) {
        // LOG("Events: %" PRIu32 ", System: %" PRIu32 ".", ++u, i + 1);

        system->event_handler[i](event);
      }
    }
  }
}

void event_handler_register(event_system* system, event_handler handle) {
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
bool event_trigger(event_queue* q, void* info, int type) {
  event* e = zmalloc(sizeof((*e)));

  e->info = info;
  e->type = type;

  return event_enqueue(q, e);
}

// bool event_destroy(event* e) {
//   event *e = zmalloc(sizeof((*e)));

//   e->info = info;
//   e->type = type;

//   return event_enqueue(q, e);
// }

bool event_enqueue(event_queue* q, event* e) {
  if (is_full(q)) return false;

  q->queue[q->tail] = e;
  q->tail = (q->tail + 1) % q->capacity;

  return true;
}
