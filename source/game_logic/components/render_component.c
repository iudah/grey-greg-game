#include "render_component.h"

#include <stdint.h>
#include <zot.h>

#include "component.h"
#include "position_component.h"
#include "simd.h"

struct render_component* render_component;

bool initialize_render_component() {
  render_component = zcalloc(1, sizeof(struct render_component));

  bool component_intialized = initialize_component(
      (struct generic_component*)render_component,
      (uint64_t[]){sizeof(*render_component->streams->color),
                   sizeof(*render_component->streams->interpolated_position)},
      sizeof(*render_component->streams) / sizeof(void*));

  // render_component->streams->color =
  //     zcalloc(MAX_NO_ENTITY, sizeof(*render_component->streams->color));

  return render_component != NULL && component_intialized;
}

bool set_entity_color(entity e, uint32_t rgba) {
  if (!has_component(e, (struct generic_component*)render_component))
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

void interpolate_positions(float interpolation_factor) {
  struct vec4_st* curr_interp_pos =
      render_component->streams->interpolated_position;

  auto ifac = vdupq_n_f32(interpolation_factor);

  for (uint32_t i = 0; i < render_component->set.count; ++i) {
    auto e = render_component->set.dense[i];

    auto p = vld1q_f32((void*)get_position(e));
    auto pprev = vld1q_f32((void*)get_previous_position(e));
    auto interp = vmlaq_f32(pprev, vsubq_f32(p, pprev), ifac);

    vst1q_f32((void*)&curr_interp_pos[i], interp);

    printf("Entity %i at (%g, %g, %g)\n", e.id, curr_interp_pos[i].x,
           curr_interp_pos[i].y, curr_interp_pos[i].z);
  }
}
