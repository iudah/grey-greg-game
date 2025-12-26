#include "force_component.h"
#include <simd.h>
#include <string.h>
#include <zot.h>

struct force_component *force_component;

bool initialize_force_component()
{
    force_component = zcalloc(1, sizeof(struct force_component));

    bool component_intialized = initialize_component(
        (struct generic_component *)force_component,
        (uint64_t[]){sizeof(*force_component->stream->force)},
        sizeof(*force_component->stream) / sizeof(void *));

    return force_component != NULL && component_intialized;
}

struct vec4_st *get_force(entity e)
{
    uint32_t dense_idx;
    if (!component_get_dense_id((struct generic_component *)force_component, e, &dense_idx))
        return NULL;

    return &force_component->stream->force[dense_idx];
}

bool apply_force(entity e, float fx, float fy, float fz)
{
    struct vec4_st *f = get_force(e);
    if (f)
    {
        auto f1 = vld1q_f32((float[]){fx, fy, fz, 0});
        auto f0 = vld1q_f32((void *)f);
        auto f2 = vaddq_f32(f1, f0);
        vst1q_f32((float *)f, f2);

        return true;
    }
    return false;
}

void clear_forces()
{
    if (force_component->set.count)
        memset(force_component->stream->force, 0, sizeof(*force_component->stream->force) * force_component->set.count);
}