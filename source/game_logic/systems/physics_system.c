#include "physics_system.h"
#include "aabb_component.h"
#include "force_component.h"
#include "mass_component.h"
#include "component.h"
#include "event_system.h"
#include "game_logic.h"
#include "game_main.h"
#include "position_component.h"
#include "simd.h"
#include "velocity_component.h"
#include "waypoint_component.h"
#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <zot.h>

float distance(struct vec4_st *npc_pos, struct vec4_st *player_pos, bool flee,
               float32x4_t *diff_ptr)
{
  auto n_pos = vld1q_f32((float *)npc_pos);
  auto p_pos = vld1q_f32((float *)player_pos);

  auto diff = flee ? vsubq_f32(n_pos, p_pos) : vsubq_f32(p_pos, n_pos);
  diff = vsetq_lane_f32(0, diff, 3); // zero 'w'

  if (diff_ptr)
  {
    memcpy(diff_ptr, &diff, sizeof(diff));
  }

  auto diff2 = vmulq_f32(diff, diff);
  auto sum = vadd_f32(vget_high_f32(diff2), vget_low_f32(diff2));
  sum = vpadd_f32(sum, sum);

  return sqrtf(vget_lane_f32(sum, 0));
}

bool aabb_overlap(struct vec4_st *a_pos, struct vec4_st *a_ext,
                  struct vec4_st *b_pos, struct vec4_st *b_ext)
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

void euler_method()
{
  struct vec4_st *prev_pos = position_component->stream->prev_position;
  struct vec4_st *pos = position_component->stream->position;
  struct vec4_st *vel = velocity_component->streams->velocity;
  //  float *mass = velocity_component->streams->velocity;

  assert(position_component->set.count <=
         position_component->set.dense_capacity);
  memcpy(prev_pos, pos, position_component->set.count * sizeof(*prev_pos));

  for (uint32_t i = 0; i < velocity_component->set.count; ++i)
  {
    entity entity = velocity_component->set.dense[i];
    uint32_t j;
    if (!component_get_dense_id((struct generic_component *)position_component, entity, &j))
      continue;

    auto t = vdupq_n_f32(TIMESTEP);
    auto v = vld1q_f32((void *)&vel[i]);

    if (has_component(entity, mass_component) && has_component(entity, force_component))
    {
      auto mass = get_mass(entity);
      if (mass < GREY_ZERO)
        mass = 1;
      auto zero_f = (struct vec4_st){{0, 0, 0, 0}};
      auto force = get_force(entity);
      if (!force)
        force = &zero_f;

      auto f = vld1q_f32((float *)force);
      auto a = vmulq_f32(f, vdupq_n_f32(1. / mass));
      v = vmlaq_f32(v, a, t);
    }
    auto p = vld1q_f32((void *)&pos[j]);
    p = vmlaq_f32(p, v, t);

    vst1q_f32((void *)&vel[i], v);
    vst1q_f32((void *)&pos[j], p);
  }
}

#if 0
void verlet_integration_method()
{
  struct vec4_st *prev_pos_arr = position_component->stream->prev_position;
  struct vec4_st *pos_arr = position_component->stream->position;
  struct vec4_st *acc_arr = (struct vec4_st *)(float[]){0, 0, 0, 0}; // velocity_component->streams->acceleration;

  auto dt = vdupq_n_f32(TIMESTEP);
  auto dt_2 = vmulq_f32(dt, dt);

  for (uint32_t i = 0; i < velocity_component->set.count; i++)
  {
    entity e = velocity_component->set.dense[i];

    uint32_t a_idx = i;
    uint32_t p_idx;

    if (!component_get_dense_id((struct generic_component *)position_component, e, &p_idx))
      continue;

    auto pcurr = vld1q_f32((float *)&pos_arr[p_idx]);
    auto pprev = vld1q_f32((float *)&prev_pos_arr[p_idx]);
    auto acc = vld1q_f32((float *)&acc_arr[a_idx]);

    auto two_pcurr = vmulq_n_f32(pcurr, 2);
    auto acc_dt2 = vmulq_f32(acc, dt_2);
    auto pnew = vsubq_f32(vaddq_f32(two_pcurr, acc_dt2), pprev);

    vst1q_f32((float *)&prev_pos_arr[p_idx], pnew);
  }

  auto temp = position_component->stream->position;
  position_component->stream->position =
      position_component->stream->prev_position;
  position_component->stream->prev_position = temp;
}
#endif

void physics_system_update()
{
  euler_method();
  // verlet_integration_method();
}

void compute_swept_aabb_box(struct vec4_st *curr_pos, struct vec4_st *prev_pos,
                            struct vec4_st *extent, float32x4_t *out_min,
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
  uint32x4_t le_max = vcleq_f32(vsubq_f32(min_a, max_b), vdupq_n_f32(GREY_ZERO));
  uint32x4_t ge_min = vcleq_f32(vsubq_f32(min_b, max_a), vdupq_n_f32(GREY_ZERO));
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

#if 1
#warning Ad-hoc velocity correction
  LOG("Ad-hoc velocity");
  // ToDo: use momentum based handler (colliding object) / AI based hander
  // (spatial aware)

  float v[4];

  float32x4_t vel;

  vel = vld1q_f32((float *)&velocity_component->streams->velocity[entity_j.id]);
  vel = vmulq_n_f32(vel, -1);
  vst1q_f32(v, vel);

  set_velocity(entity_j, v);

  vel = vld1q_f32((float *)&velocity_component->streams->velocity[entity_i.id]);
  vel = vmulq_n_f32(vel, -1);
  vst1q_f32(v, vel);

  set_velocity(entity_i, v);
#endif
}

void compute_swept_aabb_collision()
{
  float *radii = aabb_component->streams->radius;
  struct vec4_st *extents = aabb_component->streams->extent;
  // struct vec4_st *interp_positions = position_component->stream->curr_interp_position;
  // struct vec4_st *interp_prev_positions = position_component->stream->prev_interp_position;
  struct vec4_st *physix_positions = position_component->stream->position;
  struct vec4_st *physix_previous_positions = position_component->stream->prev_position;
  // struct vec4_st *prev_positions = position_component->stream->prev_interp_pos;

  if (!physix_previous_positions)
    physix_previous_positions = physix_positions;

  for (uint32_t aabb_i = 0; aabb_i < aabb_component->set.count; ++aabb_i)
  {
    entity entity_a = aabb_component->set.dense[aabb_i];
    uint32_t pos_i;

    if (!component_get_dense_id((struct generic_component *)position_component, entity_a, &pos_i))
      continue;

    float32x4_t min_a;
    float32x4_t max_a;
    compute_swept_aabb_box(&physix_positions[pos_i],
                           physix_previous_positions ? &physix_previous_positions[pos_i] : NULL,
                           &extents[aabb_i], &min_a, &max_a);

    for (uint32_t aabb_j = aabb_i + 1; aabb_j < aabb_component->set.count;
         ++aabb_j)
    {
      entity entity_b = aabb_component->set.dense[aabb_j];
      uint32_t pos_j;

      if (!component_get_dense_id(
              (struct generic_component *)position_component, entity_b, &pos_j))
        continue;

      float distance_between =
          distance(&physix_positions[pos_i], &physix_positions[pos_j], false, NULL);
      if (distance_between - (radii[aabb_i] + radii[aabb_j]) > GREY_ZERO)
      {
        continue;
      }

      float32x4_t min_b;
      float32x4_t max_b;

      compute_swept_aabb_box(&physix_positions[pos_j],
                             physix_previous_positions ? &physix_previous_positions[pos_j] : NULL,
                             &extents[aabb_j], &min_b, &max_b);

      if (check_aabb_overlap(min_a, max_a, min_b, max_b))
      {
        // ToDo:  Use Minkowski sum
        resolve_collision(entity_a, entity_b);
      }
    }
  }
}

#if 0
bool set_verlet_velocity(entity e, float *vel)
{
  set_euler_velocity(e, vel);

  uint32_t j = position_component->set.sparse[e.id];

  struct vec4_st *pos = &position_component->stream->position[j];
  struct vec4_st *prev_pos = &position_component->stream->prev_position[j];

  // Todo: Should I use the elapsed time?
  // But no time is elapsed at the start of set up
  // What if position changes mid-game, like teleportation?
  prev_pos->x = pos->x - vel[0] * TIMESTEP;
  prev_pos->y = pos->y - vel[1] * TIMESTEP;
  prev_pos->z = pos->z - vel[2] * TIMESTEP;

  return true;
}
#endif

bool set_entity_waypoint(entity e, float x, float y, float z)
{
  if (!has_component(e, (struct generic_component *)waypoint_component))
    return false;

  uint32_t j = waypoint_component->set.sparse[e.id];

  waypoint_component->streams->waypoint[j].x = x;
  waypoint_component->streams->waypoint[j].y = y;
  waypoint_component->streams->waypoint[j].z = z;

  return true;
}

bool set_entity_aabb_lim(entity e, float x, float y, float z)
{
  if (!has_component(e, (struct generic_component *)aabb_component))
    return false;

  uint32_t j = aabb_component->set.sparse[e.id];

  // half extents
  aabb_component->streams->extent[j].x = x;
  aabb_component->streams->extent[j].y = y;
  aabb_component->streams->extent[j].z = z;

  aabb_component->streams->radius[j] = sqrtf(x * x + y * y + z * z);

  return true;
}

struct vec4_st *get_next_patrol_point(entity e)
{
  if (!has_component(e, (struct generic_component *)waypoint_component))
    return NULL;

  uint32_t j = waypoint_component->set.sparse[e.id];

  return &waypoint_component->streams->waypoint[j];
}

bool advance_patrol_index(entity e)
{
  if (!has_component(e, (struct generic_component *)waypoint_component))
    return false;

  uint32_t j = waypoint_component->set.sparse[e.id];

  // TODO: Use angle of view to randomly choose waypoint

  waypoint_component->streams->waypoint[j].x++;
  waypoint_component->streams->waypoint[j].y++;
  waypoint_component->streams->waypoint[j].z++;

  return true;
}
