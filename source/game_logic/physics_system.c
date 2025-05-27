#include "../game_application/game_main.h"
#include "component.h"
#include <arm_neon.h>
#include <zot.h>

struct position_st {
  float x, y, z, w;
};
struct position_component {
  component_set set;
  struct position_st *position;
  struct position_st *prev_position;
};

struct velocity_component {
  component_set set;
  struct velocity_st {
    float x, y, z, w;
  } *velocity;
};

struct physics_component {
  component_set set;
  struct velocity_st *velocity;
  struct position_st *position;
};

struct position_component *position_component;
struct velocity_component *velocity_component;
struct physics_component *physics_component;

bool initialize_position_component() {
  position_component = zcalloc(1, sizeof(struct position_component));
  return position_component != NULL &&
         initialize_component((struct generic_component *)position_component,
                              sizeof(struct position_st));
}

bool initialize_velocity_component() {
  velocity_component = zcalloc(1, sizeof(struct velocity_component));
  return velocity_component != NULL &&
         initialize_component((struct generic_component *)velocity_component,
                              sizeof(struct velocity_st));
}

bool initialize_physics_component() {
  physics_component = zcalloc(1, sizeof(struct physics_component));
  return physics_component != NULL;
}

void static __attribute__((__constructor__(200))) init() {
  initialize_position_component();
  initialize_velocity_component();
}

void physics_system_update() {
  struct position_st *prev_pos = position_component->prev_position;
  struct position_st *pos = position_component->position;
  struct velocity_st *vel = velocity_component->velocity;

  memcpy(prev_pos, pos, position_component->set.count * sizeof(*prev_pos));

  for (uint32_t i = 0; i < velocity_component->set.count; i++) {
    uint32_t entity_id = velocity_component->set.dense[i];
    uint32_t j = position_component->set.sparse[entity_id];

    auto v = vld1q_f32((void *)&vel[i]);
    auto p = vld1q_f32((void *)&pos[j]);
    auto t = vdupq_n_f32(TIMESTEP);
    p = vmlaq_f32(p, v, t);

    vst1q_f32((void *)&pos[j], p);
  }
}