#include <zot.h>  //a custom memory manager

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
  void *event_info;
  int type;
} event;

typedef bool (*event_handler)(event *);

typedef struct {
  event_handler *event_handler;
  int *event_type;
  int no_event_handler;
  int avail_no_event_handler;
} event_system;

void initialize_event_system(event_system*system, int initial_no_of_system){
  system->event_handler=zcalloc(initial_no_of_system, sizeof(*system->event_handler));
  system->event_type=zcalloc(initial_no_of_system, sizeof(*system->event_type));
  system->no_event_handler=0;
  system->avail_no_event_handler=initial_no_of_system;
}

void event_handler_broadcast(event_system *system, event *event) {
  for (int i = 0; i < system->no_event_handler; i++) {
    if (event->type & system->event_type[i]) {
      system->event_handler[i](event);
    }
  }
}

void event_handler_register(event_system *system, event_handler handle,
                            int event_type) {
  if (!handle) {
    LOG("Event handler is null.");
    return;
  }
  if (system->no_event_handler >= MAX_EVENT_HANDLER) {
    LOG_ERROR("Maximum number of event handler reached for this broadcaster.");
    return;
  }

  system->event_handler[system->no_event_handler] = handle;
  system->event_type[system->no_event_handler] = event_type;
  system->no_event_handler++;
}