#include "render_component.h"
#include "component.h"
#include <zot.h>

struct render_component *render_component;

bool initialize_render_component() {
  render_component = zcalloc(1, sizeof(struct render_component));
  render_component->color =
      zcalloc(MAX_NO_ENTITY, sizeof(*render_component->color));

  return render_component != NULL &&
         initialize_component((struct generic_component *)render_component,
                              sizeof(struct vec4_st));
}

bool set_entity_color(entity e, uint32_t rgba) {
  if (!has_component(e, (struct generic_component *)render_component))
    return false;

  if (rgba <= 0xffffff) {
    rgba = (rgba << 0x8) | 0xff;
  }

  render_component->color[e.id].w = rgba & 0xff;
  render_component->color[e.id].z = (rgba >> 0x08) & 0xff;
  render_component->color[e.id].y = (rgba >> 0x10) & 0xff;
  render_component->color[e.id].x = (rgba >> 0x18) & 0xff;

  return true;
}
