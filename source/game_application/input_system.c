#include "input_system.h"

#include <event_system.h>
#include <math.h>
#include <raylib.h>
#include <raylib_glue.h>
#include <simd.h>
#include <stdint.h>
#include <zot.h>

#ifndef NO_RAYLIB

controller_config controller;

static const int btn_h = 60;
static const int btn_w = 50;
static const int dir_w = 180;
static const int margin = 20;

void mark_pad_area(PadIndex i, Rectangle r) {
  if (i < PAD_COUNT) {
    controller.pad[i] = r;
  }
}

void mark_stick_area(StickIndex i, Circle c) {
  if (i < STICK_COUNT) {
    controller.stick[i] = c;
  }
}

#endif

float stick_large_radius = 0;
float action_btn_expansn = 0;

static void recalculate_soft_game_pad_layout(int sw, int sh) {
  float center_x = sw * 0.85f;
  float center_y = sh * 0.70f;

  float dpad_x = sw * 0.15f;
  float dpad_y = sh * 0.70f;

  float b_size = sh * 0.10f;
  float d_size = sh * 0.28f;

  stick_large_radius = sw * 0.475 - dpad_x;
  action_btn_expansn = sw * 0.005f;

  // Right Side - Action Buttons
  mark_pad_area(PAD_A, (Rectangle){center_x + b_size, center_y, b_size, b_size});
  mark_pad_area(PAD_X, (Rectangle){center_x - b_size, center_y, b_size, b_size});
  mark_pad_area(PAD_B, (Rectangle){center_x, center_y + b_size, b_size, b_size});
  mark_pad_area(PAD_O, (Rectangle){center_x, center_y - b_size, b_size, b_size});

  mark_stick_area(STICK_BASE, (Circle){dpad_x, dpad_y, d_size / 2});
  mark_stick_area(STICK_KNOB, (Circle){dpad_x, dpad_y, b_size / 2});
}

void on_screen_resize(event *e) {
  if (e->type != SCREEN_ORIENTATION_CHANGED_EVENT) return;
  uint32_t *screensize = e->info;
  recalculate_soft_game_pad_layout(screensize[0], screensize[1]);
}

#ifndef NO_RAYLIB
static bool (*const keyaction[])(int key) = {IsKeyDown, IsKeyReleased};
static const event_type eventaction[] = {KEY_DOWN_EVENT, KEY_RELEASED_EVENT};
enum { G_KEY_LEFT, G_KEY_RIGHT, G_KEY_UP, G_KEY_DOWN } GKeyEnum;
static const KeyboardKey key[] = {KEY_LEFT, KEY_RIGHT, KEY_UP, KEY_DOWN};
static const GamepadButton pad_button[] = {[PAD_A] = GAMEPAD_BUTTON_RIGHT_FACE_UP,
                                           [PAD_O] = GAMEPAD_BUTTON_RIGHT_FACE_RIGHT,
                                           [PAD_X] = GAMEPAD_BUTTON_RIGHT_FACE_DOWN,
                                           [PAD_B] = GAMEPAD_BUTTON_RIGHT_FACE_LEFT};
#endif

Vector2 project_pt_x(Vector2 center, Vector2 pt, float radius) {
  float32x2_t v_pt = vld1_f32(((float[]){pt.x - center.x, pt.y - center.y}));
  float32x2_t v_rad = vld1_f32(((float[]){radius, radius}));

  float32x2_t result = vdiv_f32(v_pt, v_rad);

  Vector2 out;
  vst1_f32((float *)&out, result);
  return out;
}

#define IS_SET(x, n)    (x & (1 << (n)))
#define SET_BIT(x, n)   (x |= (1 << (n)))
#define CLEAR_BIT(x, n) (x &= ~(1 << (n)))

void update_input_system(game_logic *logic) {
  event_system *events_system = game_logic_get_event_system(logic);

#ifndef NO_RAYLIB
#if defined(__ANDROID__)
  int no_of_touch = GetTouchPointCount();

  static char previous_trigger = 0;
  char current_trigger = 0;
  static char previous_action = 0;
  char current_action = 0;

  Vector2 base_center = *(Vector2 *)&stick[STICK_BASE];
  Vector2 knob_center = *(Vector2 *)&stick[STICK_KNOB];
  for (int i = 0; i < no_of_touch; ++i) {
    Vector2 pt = GetTouchPosition(i);
    // if (CheckCollisionPointCircle(pt, knob_center, stick[STICK_KNOB].radius)) {
    //   current_trigger |= previous_trigger;
    //   continue;
    // }

    if (CheckCollisionPointCircle(
            pt, base_center, previous_trigger ? stick_large_radius : stick[STICK_BASE].radius)) {
      Vector2 pt_base = {pt.x - base_center.x, pt.y - base_center.y};
      float pt_radius = hypotf(pt_base.x, pt_base.y);
      float radius = stick[STICK_BASE].radius;
      if (pt_radius >= radius) {
        *(Vector2 *)&stick[STICK_KNOB] =
            (Vector2){base_center.x + (pt_base.x * radius) / pt_radius,
                      base_center.y + (pt_base.y * radius) / pt_radius};
      } else {
        *(Vector2 *)&stick[STICK_KNOB] = pt;
      }

      Vector2 pad_pt =
          project_pt_x(base_center, *(Vector2 *)&stick[STICK_KNOB], stick[STICK_BASE].radius);

      if (pad_pt.x > 0.25) SET_BIT(current_trigger, G_KEY_RIGHT);
      if (pad_pt.x < -0.25) SET_BIT(current_trigger, G_KEY_LEFT);
      if (pad_pt.y < -0.25) SET_BIT(current_trigger, G_KEY_UP);
      if (pad_pt.y > 0.25) SET_BIT(current_trigger, G_KEY_DOWN);
    }

    if (CheckCollisionPointRec(pt, pad[PAD_A])) SET_BIT(current_action, PAD_A);
    if (CheckCollisionPointRec(pt, pad[PAD_B])) SET_BIT(current_action, PAD_B);
    if (CheckCollisionPointRec(pt, pad[PAD_X])) SET_BIT(current_action, PAD_X);
    if (CheckCollisionPointRec(pt, pad[PAD_O])) SET_BIT(current_action, PAD_O);
  }

  for (int i = 0; i < 4; ++i) {
    // Flood event while held (IsKeyDown)
    if (IS_SET(current_trigger, i)) event_trigger(events_system, (void *)&key[i], KEY_DOWN_EVENT);
    if (!IS_SET(current_trigger, i) && IS_SET(previous_trigger, i))
      event_trigger(events_system, (void *)&key[i], KEY_RELEASED_EVENT);

    // Edge trigger (IsKeyPressed)
    if (IS_SET(current_action, i) && !IS_SET(previous_action, i)) {
      pad[i].height += action_btn_expansn;
      pad[i].width += action_btn_expansn;
      pad[i].x += action_btn_expansn / 2;
      pad[i].y += action_btn_expansn / 2;
    }

    // Flood event while held (IsKeyDown)
    if (IS_SET(current_action, i)) {
      event_trigger(events_system, (void *)&pad_button[i], KEY_DOWN_EVENT);
    }

    if (!IS_SET(current_action, i) && IS_SET(previous_action, i)) {
      event_trigger(events_system, (void *)&pad_button[i], KEY_RELEASED_EVENT);
      pad[i].height -= action_btn_expansn;
      pad[i].width -= action_btn_expansn;
      pad[i].x -= action_btn_expansn / 2;
      pad[i].y -= action_btn_expansn / 2;
    }
  }

  if (current_trigger == 0) {
    *(Vector2 *)&stick[STICK_KNOB] = (Vector2){base_center.x, base_center.y};
  }
  previous_trigger = current_trigger;
  previous_action = current_action;

#else
  for (uint32_t i = 0; i < sizeof(keyaction) / sizeof(*keyaction); ++i) {
    for (uint32_t j = 0; j < sizeof(key) / sizeof(*key); ++j) {
      if (keyaction[i](key[j])) {
        LOG("                                          Keyboard triggered.");
        event_trigger(events_system, (void *)&key[j], eventaction[i]);
      }
    }
  }
#endif
#endif
}
