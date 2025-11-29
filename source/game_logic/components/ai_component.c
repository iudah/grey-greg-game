#include "ai_component.h"
#include <zot.h>

struct ai_component *ai_component;

bool initialize_ai_component() {
  ai_component = zcalloc(1, sizeof(struct ai_component));
  return ai_component != NULL &&
         initialize_component((struct generic_component *)ai_component,
                              sizeof(ai_state));
}
