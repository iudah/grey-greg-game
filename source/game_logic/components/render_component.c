#include "render_component.h"

#include <stdint.h>
#include <zot.h>

#include "component.h"
#include "position_component.h"
#include "simd.h"

struct render_component *render_component;

COMPONENT_STREAM_DEFINE(render, {
  // struct vec4_st *scale;
  // struct vec4_st *rotation;
  struct vec4_st *color;
  struct vec4_st *interpolated_position;
});

bool initialize_render_component() {
  render_component = zcalloc(1, sizeof(struct render_component));

  bool component_intialized =
      initialize_component((struct generic_component *)render_component,
                           (uint64_t[]){sizeof(*render_component->streams->color),
                                        sizeof(*render_component->streams->interpolated_position)},
                           sizeof(*render_component->streams) / sizeof(void *));

  // render_component->streams->color =
  //     zcalloc(MAX_NO_ENTITY, sizeof(*render_component->streams->color));

  return render_component != NULL && component_intialized;
}

struct vec4_st *get_color(entity e) { return COMPONENT_GET(render, e, color); }

bool set_entity_color(entity e, uint32_t rgba) {
  if (!has_component(e, (struct generic_component *)render_component)) return false;

  if (rgba <= 0xffffff) {
    rgba = (rgba << 0x8) | 0xff;
  }

  struct vec4_st *color = get_color(e);

  color->w = rgba & 0xff;
  color->z = (rgba >> 0x08) & 0xff;
  color->y = (rgba >> 0x10) & 0xff;
  color->x = (rgba >> 0x18) & 0xff;

  return true;
}

void interpolate_positions(float interpolation_factor) {
  struct vec4_st *curr_interp_pos = render_component->streams->interpolated_position;

  auto ifac = vdupq_n_f32(interpolation_factor);

  for (uint32_t i = 0; i < render_component->set.count; ++i) {
    auto e = render_component->set.dense[i];

    auto pos = get_position(e);
    if (!pos) continue;
    auto p = vld1q_f32((void *)pos);

    auto prevpos = get_previous_position(e);
    if (!prevpos) prevpos = pos;
    auto pprev = vld1q_f32((void *)prevpos);

    auto interp = vmlaq_f32(pprev, vsubq_f32(p, pprev), ifac);

    vst1q_f32((void *)&curr_interp_pos[i], interp);

    // printf("Entity %i at (%g, %g, %g)\n", e.id, curr_interp_pos[i].x,
    //        curr_interp_pos[i].y, curr_interp_pos[i].z);
  }
}

struct vec4_st *get_interpolated_position(entity e) {
  return COMPONENT_GET(render, e, interpolated_position);
}
