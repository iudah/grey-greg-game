#include "human_view.h"

#include <game_logic.h>
#include <game_view.h>
#include <raylib_glue.h>
#include <render_system.h>
#include <zot.h>

struct human_view {
  void *camera_and_stuff;
};

void human_view_create(struct human_view **data) { *data = zmalloc(sizeof(struct human_view)); }
void human_view_destroy(struct human_view *data) {
  // remove data content when availavle
  zfree(data);
}

static void window_change_update(game_logic *logic) {
#ifndef NO_RAYLIB
  int screen_width = get_screen_width();
  int screen_height = get_screen_height();

  if (screen_width <= 0 || screen_height <= 0) return;

  static int last_screen_width = 0;
  static int last_screen_height = 0;

  if (last_screen_width != screen_width || last_screen_height != screen_height) {
    if (last_screen_width != 0) {
      event_trigger(
          game_logic_get_event_system(logic),
          (uint32_t[]){screen_width, screen_height, last_screen_width, last_screen_height},
          SCREEN_SIZE_CHANGED_EVENT);
#if defined(__ANDROID__)
      event_trigger(
          game_logic_get_event_system(logic),
          (uint32_t[]){screen_width, screen_height, last_screen_width, last_screen_height},
          SCREEN_ORIENTATION_CHANGED_EVENT);
#endif
    }
    last_screen_width = screen_width;
    last_screen_height = screen_height;
  }
#endif
}

void human_view_render(struct human_view *view, game_logic *logic, float interpolation_factor) {
  window_change_update(logic);

#ifndef NO_RAYLIB
  BeginDrawing();
  ClearBackground(RAYWHITE);

  render_scene(interpolation_factor);

  // draw ui elements

#if defined(__ANDROID__) && !defined(NO_RAYLIB)
  render_controller();
#endif

  EndDrawing();
#endif
}

// Todo: Update human view

game_view_class human_view_st = {.create = (game_view_create_fn)human_view_create,
                                 .render = (game_view_render_fn)human_view_render,
                                 .destroy = (game_view_destroy_fn)human_view_destroy,
                                 .update = NULL};
game_view_class *human_view = &human_view_st;
