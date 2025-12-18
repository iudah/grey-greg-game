#include "render_component.h"
#include "component.h"
#include <stdint.h>
#include <zot.h>

struct render_component *render_component;

bool initialize_render_component() {
  render_component = zcalloc(1, sizeof(struct render_component));

  bool component_intialized = initialize_component(
      (struct generic_component *)render_component,
      (uint64_t[]){sizeof(*render_component->streams->color)},
      sizeof(*render_component->streams) / sizeof(void *));

  // render_component->streams->color =
  //     zcalloc(MAX_NO_ENTITY, sizeof(*render_component->streams->color));

  return render_component != NULL && component_intialized;
}

bool set_entity_color(entity e, uint32_t rgba) {
  if (!has_component(e, (struct generic_component *)render_component))
    return false;

  if (rgba <= 0xffffff) {
    rgba = (rgba << 0x8) | 0xff;
  }

  auto color = render_component->streams->color;

  color[e.id].w = rgba & 0xff;
  color[e.id].z = (rgba >> 0x08) & 0xff;
  color[e.id].y = (rgba >> 0x10) & 0xff;
  color[e.id].x = (rgba >> 0x18) & 0xff;

  return true;
}
