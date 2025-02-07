#include <signal.h>
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

#define TIMESTEP 0.008333333333

void quit_game(int signal);
void game_cleanup();
static inline void register_interrupt_signal_handler();
static inline bool get_time_now(ztimespec *ts);
static inline double compute_lapsed_time();

bool quit = false;
ztimespec iter_start, iter_end;

int game_main() {
  atexit(game_cleanup);
  register_interrupt_signal_handler();
  get_win32_frequency();

  LOG("Game started.");

  if (!get_time_now(&iter_start)) {
    LOG_ERROR("Failed to get time.");
  }
  double time_elapsed = 0;
  double dt = 0;

  while (!quit) {

    time_elapsed = compute_lapsed_time();

    // catch up on missed time
    while (time_elapsed >= TIMESTEP) {
      LOG("Progressing game state after time %fms.", TIMESTEP * 1000.);
      time_elapsed -= TIMESTEP;
    }

    dt = compute_lapsed_time();

    get_time_now(&iter_start);

    LOG("Rendering at dt = %fms.", dt * 1000);
  }

  return 0;
}

#ifdef _WIN32
bool WINAPI console_handler(DWORD signal) {
  if (signal == CTRL_C_EVENT) {
    quit_game(0);
    return true;
  }
  return false;
}
static inline void get_win32_frequency() {
  LARGE_INTEGER f;
  QueryPerformanceFrequency(&f);
  freq = (double)f.QuadPart;
}
#endif

void register_interrupt_signal_handler() {
#ifdef _WIN32
  SetConsoleCtrlHandler(console_handler, true);
#else
  signal(SIGINT, quit_game);
#endif
}

static inline bool get_time_now(ztimespec *ts) {
#ifdef _WIN32
  QueryPerformanceCounter(ts);
  return true;
#else
  return clock_gettime(CLOCK_MONOTONIC, ts);
#endif
}

static inline double compute_lapsed_time() {
  get_time_now(&iter_end);
#ifdef _WIN32
  return (iter_end.QuadPart - iter_start.QuadPart) / freq;
#else
  return (iter_end.tv_sec - iter_start.tv_sec) +
         (iter_end.tv_nsec - iter_start.tv_nsec) / 1e9;
#endif
}

void quit_game(int signal) {
  LOG("Game ended by %d.", signal);
  quit = true;
}

void game_cleanup() { LOG("Cleaning up game resources."); }

#define MAX_EVENT_HANDLER 25

typedef struct {
  void *event_info;
  int type;
} event;

typedef bool (*event_handler)(event *);

typedef struct {
  event_handler event_handler[MAX_EVENT_HANDLER];
  int event_type[MAX_EVENT_HANDLER];
  int no_event_handler;
} event_system;

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