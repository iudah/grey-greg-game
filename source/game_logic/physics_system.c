#include "component.h"
#include "event_system.h"
#include "game_main.h"
#include "simd.h"
#include <assert.h>
#include <math.h>
#include <string.h>
#include <zot.h>

struct vec4_st
{
  float x, y, z, w;
};

struct position_component
{
  component_set set;
  struct vec4_st *position;
  struct vec4_st *prev_position;
  struct vec4_st *curr_position;
};

struct velocity_component
{
  component_set set;
  struct velocity_st
  {
    float x, y, z, w;
  } *velocity;
};

struct aabb_component
{
  component_set set;
  struct aabb_st
  {
    float x, y, z, _unused;
    float radius;
  } *extent;
  struct vec4_st *prev_timestep_pos;
};

struct waypoint_component
{
  component_set set;
  struct waypoint_st
  {
    float x, y, z, w;
  } *waypoint;
};

struct position_component *position_component;
struct velocity_component *velocity_component;
struct aabb_component *aabb_component;
struct waypoint_component *waypoint_component;

float distance(struct vec4_st *npc_pos, struct vec4_st *player_pos, bool flee,
               float32x4_t *diff_ptr);

bool initialize_position_component()
{
  position_component = zcalloc(1, sizeof(struct position_component));
  position_component->position =
      zcalloc(MAX_NO_ENTITY, sizeof(*position_component->position));
  position_component->prev_position =
      zcalloc(MAX_NO_ENTITY, sizeof(*position_component->prev_position));

  return position_component != NULL &&
         initialize_component((struct generic_component *)position_component,
                              sizeof(struct vec4_st));
}

bool initialize_velocity_component()
{
  velocity_component = zcalloc(1, sizeof(struct velocity_component));
  velocity_component->velocity =
      zcalloc(MAX_NO_ENTITY, sizeof(*velocity_component->velocity));
  return velocity_component != NULL &&
         initialize_component((struct generic_component *)velocity_component,
                              sizeof(struct velocity_st));
}

bool initialize_aabb_component()
{
  aabb_component = zcalloc(1, sizeof(struct aabb_component));
  aabb_component->extent =
      zcalloc(MAX_NO_ENTITY, sizeof(*aabb_component->extent));
  return aabb_component != NULL &&
         initialize_component((struct generic_component *)aabb_component,
                              sizeof(struct aabb_st));
}

bool initialize_waypoint_component()
{
  waypoint_component = zcalloc(1, sizeof(struct waypoint_component));
  waypoint_component->waypoint =
      zcalloc(MAX_NO_ENTITY, sizeof(*waypoint_component->waypoint));
  return waypoint_component != NULL &&
         initialize_component((struct generic_component *)waypoint_component,
                              sizeof(struct waypoint_st));
}

void static __attribute__((constructor(200))) init()
{
  initialize_position_component();
  initialize_velocity_component();
  initialize_aabb_component();
  initialize_waypoint_component();
}

bool aabb_overlap(struct vec4_st *a_pos, struct aabb_st *a_ext,
                  struct vec4_st *b_pos, struct aabb_st *b_ext)
{

  auto a_pos_simd = vld1q_f32((float *)a_pos);
  auto b_pos_simd = vld1q_f32((float *)b_pos);
  auto a_ext_simd = vld1q_f32((float *)a_ext);
  auto b_ext_simd = vld1q_f32((float *)b_ext);

  auto pos_diff = vabsq_f32(vsubq_f32(a_pos_simd, b_pos_simd));
  auto ext_sum = vaddq_f32(a_ext_simd, b_ext_simd);

  auto l_eq = vcleq_f32(pos_diff, ext_sum);

  return vgetq_lane_s32(l_eq, 0) * vgetq_lane_s32(l_eq, 1) *
             vgetq_lane_s32(l_eq, 2) >
         0;
}

void physics_system_update()
{
  struct vec4_st *prev_pos = position_component->prev_position;
  struct vec4_st *pos = position_component->position;
  struct velocity_st *vel = velocity_component->velocity;
  struct aabb_st *aabb = aabb_component->extent;

  assert(position_component->set.count <= MAX_NO_ENTITY);
  memcpy(prev_pos, pos, position_component->set.count * sizeof(*prev_pos));

  for (uint32_t i = 0; i < velocity_component->set.count; ++i)
  {
    entity entity_id = velocity_component->set.dense[i];
    if (!has_component(entity_id, position_component))
      continue;
    uint32_t j = position_component->set.sparse[entity_id.id];

    auto v = vld1q_f32((void *)&vel[i]);
    auto p = vld1q_f32((void *)&pos[j]);
    auto t = vdupq_n_f32(TIMESTEP);
    p = vmlaq_f32(p, v, t);

    vst1q_f32((void *)&pos[j], p);
  }

  //  min = position - extent, max = position + extent

  // for (uint32_t i = 0; i < aabb_component->set.count; ++i) {
  //   entity entity_i = aabb_component->set.dense[i];
  //   uint32_t pos_idx_i = position_component->set.sparse[entity_i.id];

  //   for (uint32_t j = i; j < aabb_component->set.count; j++) {
  //     entity entity_j = aabb_component->set.dense[j];
  //     uint32_t pos_idx_j = position_component->set.sparse[entity_j.id];

  //     float gap = distance(&pos[pos_idx_i], &pos[pos_idx_j], false, NULL);
  //     if (gap > aabb[i].radius && gap > aabb[j].radius) {
  //       continue;
  //     }

  // check overlap
  // if (aabb_overlap(&pos[pos_idx_i], &aabb[i], &pos[pos_idx_j], &aabb[j]))
  // { collision detected
  // }
  //   }
  // }
}

void compute_swept_aabb_box(struct vec4_st *curr_pos, struct vec4_st *prev_pos,
                            struct aabb_st *extent, float32x4_t *out_min,
                            float32x4_t *out_max)
{

  if (!prev_pos)
  {
    prev_pos = curr_pos;
  }

  auto prev = vld1q_f32((float *)prev_pos);
  auto curr = vld1q_f32((float *)curr_pos);

  auto half_extent = vld1q_f32((float *)extent);

  // Ensure W component is zero (unused)
  vsetq_lane_f32(0.f, half_extent, 3);

  auto swept_min =
      vminq_f32(vsubq_f32(prev, half_extent), vsubq_f32(curr, half_extent));

  auto swept_max =
      vmaxq_f32(vaddq_f32(prev, half_extent), vaddq_f32(curr, half_extent));

  if (out_min)
  {
    memcpy(out_min, &swept_min, sizeof(swept_min));
  }
  if (out_max)
  {
    memcpy(out_max, &swept_max, sizeof(swept_max));
  }
}

bool check_aabb_overlap(float32x4_t min_a, float32x4_t max_a, float32x4_t min_b,
                        float32x4_t max_b)
{
  uint32x4_t le_max = vcleq_f32(min_a, max_b);
  uint32x4_t ge_min = vcleq_f32(min_b, max_a);
  uint32x4_t overlap_mask = vandq_u32(le_max, ge_min);

  uint32_t result[4];
  vst1q_u32(result, overlap_mask);

  // Only check x, y, z (ignore w)
  return result[0] && result[1] && result[2];
}

void resolve_collision(entity entity_i, entity entity_j)
{

  event_enqueue_collision(entity_i, entity_j);

  LOG("Collision between entity %d and %d", entity_i.id, entity_j.id);

#warning Ad-hoc velocity correction
  LOG("Ad-hoc velocity");
  // ToDo: use momentum based handler (colliding object) / AI based hander
  // (spatial aware)
  float32x4_t vel;

  vel = vld1q_f32((float *)&velocity_component->velocity[entity_j.id]);
  vel = vmulq_n_f32(vel, -1);
  vst1q_f32((float *)&velocity_component->velocity[entity_j.id], vel);

  vel = vld1q_f32((float *)&velocity_component->velocity[entity_i.id]);
  vel = vmulq_n_f32(vel, -1);
  vst1q_f32((float *)&velocity_component->velocity[entity_i.id], vel);
}

void compute_swept_aabb_collision()
{
  struct aabb_st *extents = aabb_component->extent;
  struct vec4_st *curr_positions = position_component->curr_position;
  struct vec4_st *prev_positions = aabb_component->prev_timestep_pos;

  for (uint32_t i = 0; i < aabb_component->set.count; ++i)
  {
    entity entity_a = aabb_component->set.dense[i];

    if (!has_component(entity_a, position_component))
      continue;
    uint32_t idx_a = position_component->set.sparse[entity_a.id];

    float32x4_t min_a;
    float32x4_t max_a;
    compute_swept_aabb_box(&curr_positions[idx_a],
                           prev_positions ? &prev_positions[idx_a] : NULL,
                           &extents[i], &min_a, &max_a);

    for (uint32_t j = i + 1; j < aabb_component->set.count; j++)
    {
      entity entity_b = aabb_component->set.dense[j];

      if (!has_component(entity_b, position_component))
        continue;
      uint32_t idx_b = position_component->set.sparse[entity_b.id];

      float distance_between =
          distance(&curr_positions[idx_a], &curr_positions[idx_b], false, NULL);
      if (distance_between > (extents[i].radius + extents[j].radius))
      {
        continue;
      }

      float32x4_t min_b;
      float32x4_t max_b;
      compute_swept_aabb_box(&curr_positions[idx_b],
                             prev_positions ? &prev_positions[idx_b] : NULL,
                             &extents[j], &min_b, &max_b);

      if (check_aabb_overlap(min_a, max_a, min_b, max_b))
      {
        resolve_collision(entity_a, entity_b);
      }
    }
  }
}

void interpolate_positions(float interpolation_factor)
{

  void *tmp = aabb_component->prev_timestep_pos;
  if (!tmp)
    tmp = zmalloc(MAX_NO_ENTITY * sizeof(struct vec4_st));

  aabb_component->prev_timestep_pos = position_component->curr_position;
  position_component->curr_position = tmp;

  struct vec4_st *prev_pos = position_component->prev_position;
  struct vec4_st *pos = position_component->position;
  struct vec4_st *curr_pos = position_component->curr_position;

  struct aabb_st *aabb = aabb_component->extent;

  auto ifac = vdupq_n_f32(interpolation_factor);

  for (uint32_t i = 0; i < position_component->set.count; ++i)
  {

    auto p = vld1q_f32((void *)&pos[i]);
    auto pprev = vld1q_f32((void *)&prev_pos[i]);
    auto interp = vmlaq_f32(pprev, vsubq_f32(p, pprev), ifac);

    vst1q_f32((void *)&curr_pos[i], interp);

    entity entity_i = position_component->set.dense[i];
    printf("Entity %i at (%g, %g, %g)\n", entity_i.id, curr_pos[i].x,
           curr_pos[i].y, curr_pos[i].z);
  }
}

struct vec4_st *get_position(entity e)
{
  if (!has_component(e, (struct generic_component *)position_component))
    return NULL;

  uint32_t j = position_component->set.sparse[e.id];

  return &position_component->position[j];
}

struct velocity_st *get_velocity(entity e)
{
  if (!has_component(e, (struct generic_component *)velocity_component))
    return NULL;

  uint32_t j = velocity_component->set.sparse[e.id];

  return &velocity_component->velocity[j];
}

bool set_velocity(entity e, float *vel)
{
  if (!has_component(e, (struct generic_component *)velocity_component))

    return false;

  uint32_t j = velocity_component->set.sparse[e.id];

  velocity_component->velocity[j].x = vel[0];
  velocity_component->velocity[j].y = vel[1];
  velocity_component->velocity[j].z = vel[2];

  return true;
}

bool set_entity_velocity(entity e, float x, float y, float z)
{
  return set_velocity(e, (float[]){x, y, z});
}

bool set_entity_waypoint(entity e, float x, float y, float z)
{
  if (!has_component(e, (struct generic_component *)waypoint_component))
    return false;

  uint32_t j = waypoint_component->set.sparse[e.id];

  waypoint_component->waypoint[j].x = x;
  waypoint_component->waypoint[j].y = y;
  waypoint_component->waypoint[j].z = z;

  return true;
}

bool set_entity_aabb_lim(entity e, float x, float y, float z)
{
  if (!has_component(e, (struct generic_component *)aabb_component))
    return false;

  uint32_t j = aabb_component->set.sparse[e.id];

  aabb_component->extent[j].x = x;
  aabb_component->extent[j].y = y;
  aabb_component->extent[j].z = z;

  aabb_component->extent[j].radius = sqrtf(x * x + y * y + z * z);

  return true;
}

bool set_entity_position(entity e, float x, float y, float z)
{
  if (!has_component(e, (struct generic_component *)position_component))
    return false;

  uint32_t j = position_component->set.sparse[e.id];

  position_component->position[j].x = x;
  position_component->position[j].y = y;
  position_component->position[j].z = z;

  position_component->prev_position[j].x = x;
  position_component->prev_position[j].y = y;
  position_component->prev_position[j].z = z;

  return true;
}

struct waypoint_st *get_next_patrol_point(entity e)
{
  if (!has_component(e, (struct generic_component *)waypoint_component))
    return NULL;

  uint32_t j = waypoint_component->set.sparse[e.id];

  return &waypoint_component->waypoint[j];
}

bool advance_patrol_index(entity e)
{
  if (!has_component(e, (struct generic_component *)waypoint_component))
    return false;

  uint32_t j = waypoint_component->set.sparse[e.id];

  // TODO: Use angle of view to randomly choose waypoint

  waypoint_component->waypoint[j].x++;
  waypoint_component->waypoint[j].y++;
  waypoint_component->waypoint[j].z++;

  return true;
}

// struct physics_component {
//   component_set set;
//   struct velocity_st *velocity;
//   struct vec4_st *position;
// };

// struct physics_component *physics_component;
// bool initialize_physics_component() {
//   physics_component = zcalloc(1, sizeof(struct physics_component));
//   return physics_component != NULL;
// }