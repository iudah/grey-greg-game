#include "input_system.h"

#include <event_system.h>
#include <raylib_glue.h>
#include <stdint.h>
#include <zot.h>

#ifndef NO_RAYLIB
static bool (*const keyaction[])(int key) = {IsKeyDown, IsKeyReleased};
static const event_type eventaction[] = {KEY_DOWN_EVENT, KEY_RELEASED_EVENT};
static const KeyboardKey key[] = {KEY_LEFT, KEY_RIGHT, KEY_UP, KEY_DOWN};
#endif

void update_input_system() {
#ifndef NO_RAYLIB
  for (uint32_t i = 0; i < sizeof(keyaction) / sizeof(*keyaction); ++i) {
    for (uint32_t j = 0; j < sizeof(key) / sizeof(*key); ++j) {
      if (keyaction[i](key[j])) {
        LOG("                                          Keyboard triggered.");
        event_trigger(event__queue, &key[j], eventaction[i]);
      }
    }
  }
#endif
}
