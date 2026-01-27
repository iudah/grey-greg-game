#ifndef GAME_VIEW_H
#define GAME_VIEW_H

#include <game_logic.h>

typedef struct game_view game_view;

typedef game_view *(*game_view_create_fn)(void **view_data);
typedef void (*game_view_update_fn)(void *view_data, game_logic *logic, float dt);
typedef void (*game_view_render_fn)(void *view_data, game_logic *logic, float interpolation_factor);
typedef void (*game_view_destroy_fn)(void *view_data);

typedef struct {
  game_view_create_fn create;
  game_view_render_fn render;
  game_view_update_fn update;
  game_view_destroy_fn destroy;
} game_view_class;

void game_view_render(game_view *view, game_logic *logic, float interpolation_factor);
game_view *game_view_create(game_view_class *class);
void game_view_destroy(game_view *view);

#endif