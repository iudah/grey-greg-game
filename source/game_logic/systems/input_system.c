#include "input_system.h"

#include <event_system.h>
#include <raylib.h>
#include <stdint.h>
#include <zot.h>

static bool (*const keyaction[])(int key) = {IsKeyDown, IsKeyReleased};
static const event_type eventaction[] = {KEY_DOWN_EVENT, KEY_RELEASED_EVENT};
static const KeyboardKey key[] = {KEY_LEFT, KEY_RIGHT, KEY_UP, KEY_DOWN};

void update_input_system() {
  for (uint32_t i = 0; i < sizeof(keyaction) / sizeof(*keyaction); ++i) {
    for (uint32_t j = 0; j < sizeof(key) / sizeof(*key); ++j) {
      if (keyaction[i](key[j])) {
        LOG("                                          Keyboard triggered.");
        event_trigger(event__queue, &key[j], eventaction[i]);
      }
    }
  }
}