#include "game_main.h"

#include <game_logic.h>
#include <raylib_glue.h>
#include <signal.h>
#include <stdlib.h>
#include <zot.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <time.h>
#include <unistd.h>
#endif

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
static inline bool get_time_now(ztimespec* ts);
static inline double compute_lapsed_time();
static void render_frame(float interpolation_factor);

bool is_2d = false;

bool quit = false;
ztimespec iter_start, iter_end;

void use_2d() { is_2d = true; }

static struct {
  char* title;
  uint32_t screen_width, screen_height, FPS;
} game_config = {"Grey with Raylib", 800, 450, 60};

float TIMESTEP = 1.f / 60.f;

int game_main() {
  atexit(game_cleanup);
  register_interrupt_signal_handler();

  get_win32_frequency();

  // Initialize window
  init_window(game_config.screen_width, game_config.screen_height,
              game_config.title);
  set_FPS(game_config.FPS);

  LOG("Game started.");

  if (!get_time_now(&iter_start)) {
    LOG_ERROR("Failed to get time.");
  }
  double time_elapsed = 0;
  double dt = 0;

  while (!quit && !window_should_close()) {
    double frame_time = compute_lapsed_time();
    time_elapsed += frame_time;

// #define MAIN_EPSILON 1e-6
#define MAX_ACCUMULATED_TIME 0.25  // Avoid spiral of death
    if (time_elapsed - MAX_ACCUMULATED_TIME > GREY_ZERO) {
      time_elapsed = MAX_ACCUMULATED_TIME;
    }

    // catch up on missed time
    while (time_elapsed >= TIMESTEP) {
      // ***
      // LOG("Progressing game state after time %fms.", time_elapsed * 1000.);
      systems_update();
      time_elapsed -= TIMESTEP;

      // ***
      // LOG("Time lapsed after `systems_update();`: %fms", time_elapsed *
      // 1000.);
    }

    double interpolation_factor = time_elapsed / TIMESTEP;
    // Render logs
    // LOG("Rendering at frame_time = %fms, factor = %f.", frame_time * 1000,
    //     interpolation_factor);
    render_frame(interpolation_factor);
  }

  close_window();
  return 0;
}

void set_game_screen_config(uint32_t screen_width, uint32_t screen_height,
                            char* title, uint32_t FPS) {
  game_config.title = zstrdup(title);
  game_config.screen_height = screen_height;
  game_config.screen_width = screen_width;
  game_config.FPS = FPS;

  TIMESTEP = 1.f / FPS;
}

void render();

static void render_frame(float interpolation_factor) {
  if (interpolation_factor - 1 > GREY_ZERO) interpolation_factor = 1;
  // render
  interpolate_positions(interpolation_factor);
  render();
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

static inline bool get_time_now(ztimespec* ts) {
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
  double time_elapsed = (iter_end.tv_sec - iter_start.tv_sec) +
                        (iter_end.tv_nsec - iter_start.tv_nsec) / 1e9;
#endif
  get_time_now(&iter_start);
  return time_elapsed;
}

void quit_game(int signal) {
  LOG("Game ended by %d.", signal);
  quit = true;
}

void game_cleanup() { LOG("Cleaning up game resources."); }
