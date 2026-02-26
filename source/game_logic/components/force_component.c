#include "force_component.h"

#include <simd.h>
#include <string.h>
#include <zot.h>

struct force_component *force_component;

COMPONENT_STREAM_DEFINE(force, { struct vec4_st *force; });

bool initialize_force_component() {
  force_component = zcalloc(1, sizeof(struct force_component));

  bool component_intialized =
      initialize_component((struct generic_component *)force_component,
                           (uint64_t[]){sizeof(*force_component->streams->force)},
                           sizeof(*force_component->streams) / sizeof(void *));

  return force_component != NULL && component_intialized;
}

struct vec4_st *get_force(entity e) { return COMPONENT_GET(force, e, force); }

 bool add_force(entity e, float *force) {
  return apply_force(e, force[0], force[1], force[2]);
}

bool apply_force(entity e, float fx, float fy, float fz) {
  struct vec4_st *f = get_force(e);
  if (f) {
    float f_scaled[] = {fx, fy, fz, 0};
    auto f1 = vld1q_f32(f_scaled);
    auto f0 = vld1q_f32((void *)f);
    auto f2 = vaddq_f32(f1, f0);
    vst1q_f32((float *)f, f2);

    return true;
  }
  return false;
}

void clear_forces() {
  if (force_component->set.count)
    memset(force_component->streams->force, 0,
           sizeof(*force_component->streams->force) * force_component->set.count);
}