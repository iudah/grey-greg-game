#include "component.h"
#include "simd.h"
#include <zot.h>

struct rotation_component
{
  component_set set;
  struct rotation_st
  {
    float x, y, z, w;
  } *rotation;
};

struct scale_component
{
  component_set set;
  struct scale_st
  {
    float x, y, z, w;
  } *scale;
};

struct render_component
{
  component_set set;
  struct scale_st *scale;
  struct rotation_st *rotation;
};

struct rotation_component *rotation_component;
struct scale_component *scale_component;
struct render_component *render_component;

bool initialize_rotation_component()
{
  rotation_component = zcalloc(1, sizeof(struct rotation_component));
  return rotation_component != NULL &&
         initialize_component((struct generic_component *)rotation_component,
                              sizeof(struct rotation_st));
}

bool initialize_scale_component()
{
  scale_component = zcalloc(1, sizeof(struct scale_component));
  return scale_component != NULL;
}

bool initialize_render_component()
{
  render_component = zcalloc(1, sizeof(struct render_component));
  return render_component != NULL;
}

void static __attribute__((constructor(200))) init()
{
  initialize_rotation_component();
  initialize_scale_component();
}

void render_system_update()
{
  struct rotation_st *rot = rotation_component->rotation;
  struct scale_st *scale = scale_component->scale;

  for (uint32_t i = 0; i < scale_component->set.count; ++i)
  {
    // compute transform matrix
  }
}