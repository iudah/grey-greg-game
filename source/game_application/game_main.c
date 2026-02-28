#include "game_main.h"

#ifdef _WIN32
#include <windows.h>
#define HAS_WIN32_API 1
#else
#include <time.h>
#include <unistd.h>
#endif

#include <game_logic.h>
#include <game_view.h>
#include <grey_constants.h>
#include <human_view.h>
#include <input_system.h>
#include <raylib_glue.h>
#include <signal.h>
#include <stdlib.h>
#include <zot.h>

#ifdef _WIN32
#define signal(...)
#define zsleep(ms) Sleep(ms)
typedef LARGE_INTEGER ztimespec;
double freq;
static inline void get_win32_frequency();
#else
typedef struct timespec ztimespec;
#define get_win32_frequency(...)
#define zsleep(ms) usleep((ms) * 1000)
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

void quit_game(int signal);
void game_cleanup();
static inline void register_interrupt_signal_handler();
static inline bool get_time_now(ztimespec *ts);
static inline double compute_lapsed_time();

bool _is_2d = false;

bool quit = false;
ztimespec iter_start, iter_end;

void use_2d() { _is_2d = true; }

static struct {
  char *title;
  uint32_t screen_width, screen_height, FPS;
} game_config = {"Grey with Raylib", 800, 450, 60};

float TIMESTEP = 1.f / 60.f;

int game_main(void (*user_game)(game_logic *logic)) {
  atexit(game_cleanup);
  register_interrupt_signal_handler();

  get_win32_frequency();

  game_logic *logic = game_logic_create();
  game_view *human_game_view = game_view_create(human_view);
  // todo: implement netwoork view and ai view

#ifndef NO_RAYLIB
  // Initialize window
  init_window(game_config.screen_width, game_config.screen_height, game_config.title);
  set_FPS(game_config.FPS);
#endif

  user_game(logic);

  LOG("Game started.");

  if (!get_time_now(&iter_start)) {
    LOG_ERROR("Failed to get time.");
  }
  double time_elapsed = 0;
  double dt = 0;

  while (!quit
#ifndef NO_RAYLIB
         && !window_should_close()
#endif
  ) {
    double frame_time = compute_lapsed_time();
    time_elapsed += frame_time;

#define MAX_ACCUMULATED_TIME 0.25  // Avoid spiral of death
    if (time_elapsed - MAX_ACCUMULATED_TIME > GREY_ZERO) {
      time_elapsed = MAX_ACCUMULATED_TIME;
    }

    // catch up on missed time
    while (time_elapsed >= TIMESTEP) {
      update_input_system(logic);
      game_logic_update(logic, TIMESTEP);

      time_elapsed -= TIMESTEP;
    }

    double interpolation_factor = time_elapsed / TIMESTEP;
    game_view_render(human_game_view, logic, (float)interpolation_factor);
  }

  game_logic_destroy(logic);
  game_view_destroy(human_game_view);

#ifndef NO_RAYLIB
  close_window();
#endif
  return 0;
}

void set_game_screen_config(uint32_t screen_width, uint32_t screen_height, char *title,
                            uint32_t FPS) {
  game_config.title = zstrdup(title);
  game_config.screen_height = screen_height;
  game_config.screen_width = screen_width;
  game_config.FPS = FPS;

  TIMESTEP = 1.f / FPS;
}

#ifdef _WIN32
int WINAPI console_handler(DWORD signal) {
  if (signal == CTRL_C_EVENT) {
    quit_game(0);
    return (int)true;
  }
  return (int)false;
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
  return clock_gettime(CLOCK_MONOTONIC, ts) == 0;
#endif
}

static inline double compute_lapsed_time() {
  get_time_now(&iter_end);
#ifdef _WIN32
  double time_elapsed = (iter_end.QuadPart - iter_start.QuadPart) / freq;
#else
  double time_elapsed =
      (iter_end.tv_sec - iter_start.tv_sec) + (iter_end.tv_nsec - iter_start.tv_nsec) / 1e9;
#endif
  get_time_now(&iter_start);
  return time_elapsed;
}

void quit_game(int signal) {
  LOG("Game ended by %d.", signal);
  quit = true;
}

void game_cleanup() { LOG("Cleaning up game resources."); }
