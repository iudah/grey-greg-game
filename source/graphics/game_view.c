#include "game_view.h"

#include <raylib_glue.h>
#include <render_system.h>
#include <zot.h>

struct game_view {
  game_view_class *class;
  void *internal_data;
};

game_view *game_view_create(game_view_class *class) {
  game_view *view = zmalloc(sizeof(*view));

  class->create(&view->internal_data);
  return view;
}

void game_view_destroy(game_view *view) {
  view->class->destroy(view->internal_data);
  zfree(view);
}

void game_view_render(game_view *view, game_logic *logic, float interpolation_factor) {
  view->class->render(view->internal_data, logic, interpolation_factor);
}

void game_view_update(game_view *view, game_logic *logic, float dt) {
  view->class->render(view->internal_data, logic, dt);
}
