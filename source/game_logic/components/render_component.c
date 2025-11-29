#include "render_component.h"
#include <zot.h>

struct render_component *render_component;

bool initialize_render_component() {
  render_component = zcalloc(1, sizeof(struct render_component));
  return render_component != NULL;
}
