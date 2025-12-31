#include "physics_system.h"

#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <zot.h>

#include "aabb_component.h"
#include "component.h"
#include "event_system.h"
#include "force_component.h"
#include "game_logic.h"
#include "game_main.h"
#include "mass_component.h"
#include "position_component.h"
#include "simd.h"
#include "velocity_component.h"
#include "waypoint_component.h"

typedef struct
{
  entity a, b;
  // float collision_time_factor;
} collision_data;

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

  return vgetq_lane_s32(l_eq, 0) & vgetq_lane_s32(l_eq, 1) &
         vgetq_lane_s32(l_eq, 2) != 0;
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
    if (!component_get_dense_id((struct generic_component *)position_component,
                                entity, &j))
      continue;

    auto t = vdupq_n_f32(TIMESTEP);
    auto v = vld1q_f32((void *)&vel[i]);

    if (has_component(entity, mass_component) &&
        has_component(entity, force_component))
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
  compute_collisions();
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
  uint32x4_t le_max =
      vcleq_f32(vsubq_f32(min_a, max_b), vdupq_n_f32(GREY_ZERO));
  uint32x4_t ge_min =
      vcleq_f32(vsubq_f32(min_b, max_a), vdupq_n_f32(GREY_ZERO));
  uint32x4_t overlap_mask = vandq_u32(le_max, ge_min);

  uint32_t result[4];
  vst1q_u32(result, overlap_mask);

  // Only check x, y, z (ignore w)
  return result[0] && result[1] && result[2];
}

void event_enqueue_collision(entity entity_i, entity entity_j)
{
  collision_data *collision = zmalloc(sizeof(*collision));

  collision->a = entity_i;
  collision->b = entity_j;

  event_trigger(event__queue, collision, COLLISION_EVENT);
}

void resolve_collision(entity entity_i, entity entity_j)
{
  event_enqueue_collision(entity_i, entity_j);

  LOG("Collision between entity %d and %d", entity_i.id, entity_j.id);

#if 0
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

static inline bool radii_collide(struct vec4_st *prev_pos_a,
                                 struct vec4_st *pos_a,
                                 struct vec4_st *prev_pos_b,
                                 struct vec4_st *pos_b,
                                 float radii_a,
                                 float radii_b)
{
  auto pa = vld1q_f32((float *)prev_pos_a);
  auto pb = vld1q_f32((float *)prev_pos_b);

  auto va = vsubq_f32(vld1q_f32((float *)pos_a), pa);
  auto vb = vsubq_f32(vld1q_f32((float *)pos_b), pb);
  auto v_rel = vsubq_f32(vb, va);

  auto ba = vsubq_f32(pa, pb);

  // Zero W component to prevent garbage data
  v_rel = vsetq_lane_f32(0, v_rel, 3);
  ba = vsetq_lane_f32(0, ba, 3);

  float v_len_sq = vaddvq_f32(vmulq_f32(v_rel, v_rel));
  float dot = vaddvq_f32(vmulq_f32(ba, v_rel));

  float t = 0.0f;
  if (v_len_sq > GREY_ZERO)
  {
    t = dot / v_len_sq;
  }

  if (t < 0.0f)
  {
    t = 0;
  }
  if (t > 1.0f)
  {
    t = 1;
  }
  auto closest = vmlaq_n_f32(pb, v_rel, t);

  auto dist_vec = vsubq_f32(pa, closest);
  float dist_sq = vaddvq_f32(vmulq_f32(dist_vec, dist_vec));
  float r_sum = radii_a + radii_b;

  return dist_sq <= (r_sum * r_sum);
}

static inline float magnitude(float32x4_t v)
{
  auto v2 = vmulq_f32(v, v);
  return sqrtf(vaddvq_f32(v2));
}

bool ray_box_collision1(struct vec4_st *prev_pos_a, struct vec4_st *prev_pos_b,
                        struct vec4_st *pos_a, struct vec4_st *pos_b,
                        struct vec4_st *extents_a, struct vec4_st *extents_b,
                        float *factor, int *coll_axis)
{
  float32x4_t extent_sum =
      vaddq_f32(vld1q_f32((float *)extents_a),
                vld1q_f32((float *)extents_b));

  float32x4_t pa0 = vld1q_f32((float *)prev_pos_a);
  float32x4_t pb0 = vld1q_f32((float *)prev_pos_b);
  float32x4_t pa1 = vld1q_f32((float *)pos_a);
  float32x4_t pb1 = vld1q_f32((float *)pos_b);

  // Zero w
  pa0 = vsetq_lane_f32(0, pa0, 3);
  pb0 = vsetq_lane_f32(0, pb0, 3);
  extent_sum = vsetq_lane_f32(0, extent_sum, 3);

  float32x4_t left = vsubq_f32(pa0, extent_sum);
  float32x4_t right = vaddq_f32(pa0, extent_sum);

  // ray dir
  float32x4_t dir =
      vsubq_f32(vsubq_f32(pb1, pb0), vsubq_f32(pa1, pa0));

  float dir_arr[4];
  vst1q_f32(dir_arr, dir);

  for (int i = 0; i < 3; ++i)
  {
    if (fabsf(dir_arr[i]) <= GREY_ZERO)
    {
      if (pb0[i] < vgetq_lane_f32(left, i) || pb0[i] > vgetq_lane_f32(right, i))
        return false;
    }

    dir_arr[i] = 1.f / dir_arr[i];
  }
  dir_arr[3] = 0;

  float32x4_t inv_dir = vld1q_f32(dir_arr);

  float32x4_t t0 =
      vmulq_f32(vsubq_f32(left, pb0), inv_dir);
  float32x4_t t1 =
      vmulq_f32(vsubq_f32(right, pb0), inv_dir);

  float32x4_t tmin = vminq_f32(t0, t1);
  float32x4_t tmax = vmaxq_f32(t0, t1);

  float tmin_s[4], tmax_s[4];
  vst1q_f32(tmin_s, tmin);
  vst1q_f32(tmax_s, tmax);

  float t_enter = -INFINITY;
  int axis = 0;
  for (int i = 0; i < 3; i++)
  {
    if (tmin_s[i] > t_enter)
    {
      t_enter = tmin_s[i];
      axis = i;
    }
  }

  fmaxf(tmin_s[0], fmaxf(tmin_s[1], tmin_s[2]));
  float t_exit = fminf(tmax_s[0], fminf(tmax_s[1], tmax_s[2]));

  if (t_exit < 0.0f || t_enter > t_exit || t_enter > 1.0f)
    return false;

  if (factor)
    *factor = t_enter;

  if (coll_axis)
    *coll_axis = axis;

  return true;
}

bool ray_box_collision(struct vec4_st *prev_pos_a, struct vec4_st *prev_pos_b,
                       struct vec4_st *pos_a, struct vec4_st *pos_b,
                       struct vec4_st *extents_a, struct vec4_st *extents_b,
                       float *factor, int *coll_axis)
{
  float32x4_t pa0 = vld1q_f32((float *)prev_pos_a);
  float32x4_t pb0 = vld1q_f32((float *)prev_pos_b);
  float32x4_t pa1 = vld1q_f32((float *)pos_a);
  float32x4_t pb1 = vld1q_f32((float *)pos_b);

  // Calculate relative movement
  float32x4_t ds_a = vsubq_f32(pa1, pa0);
  float32x4_t ds_b = vsubq_f32(pb1, pb0);
  float32x4_t ds = vsubq_f32(ds_b, ds_a);

  // Early exit if no relative movement
  uint32x4_t rel_motion_mask = vcltq_f32(vabsq_f32(ds), vdupq_n_f32(GREY_ZERO));

  if (vgetq_lane_u32(rel_motion_mask, 0) == 0 &&
      vgetq_lane_u32(rel_motion_mask, 1) == 0 &&
      (!is_2d && vgetq_lane_u32(rel_motion_mask, 2) == 0))
    return false;

  float32x4_t extent_sum =
      vaddq_f32(vld1q_f32((float *)extents_a),
                vld1q_f32((float *)extents_b));

  // Zero w
  pa0 = vsetq_lane_f32(0, pa0, 3);
  pb0 = vsetq_lane_f32(0, pb0, 3);
  extent_sum = vsetq_lane_f32(0, extent_sum, 3);

  float32x4_t left = vsubq_f32(pa0, extent_sum);
  float32x4_t right = vaddq_f32(pa0, extent_sum);

  // ray dir
  float32x4_t dir =
      vsubq_f32(vsubq_f32(pb1, pb0), vsubq_f32(pa1, pa0));

  float dir_arr[4];
  vst1q_f32(dir_arr, dir);

  for (int i = 0; i < 3; ++i)
  {
    if (fabsf(dir_arr[i]) > GREY_ZERO)
      dir_arr[i] = 1.f / dir_arr[i];
    else
    {
      if (pb0[i] < vgetq_lane_f32(left, i) || pb0[i] > vgetq_lane_f32(right, i))
        return false;

      dir_arr[i] = (dir_arr[i] < 0) ? -INFINITY : INFINITY;
    }
  }
  dir_arr[3] = 0;

  float32x4_t inv_dir = vld1q_f32(dir_arr);

  float32x4_t t0 =
      vmulq_f32(vsubq_f32(left, pb0), inv_dir);
  float32x4_t t1 =
      vmulq_f32(vsubq_f32(right, pb0), inv_dir);

  float32x4_t tmin = vminq_f32(t0, t1);
  float32x4_t tmax = vmaxq_f32(t0, t1);

  float tmin_s[4], tmax_s[4];
  vst1q_f32(tmin_s, tmin);
  vst1q_f32(tmax_s, tmax);

  float t_enter = -INFINITY;
  int axis = 0;
  for (int i = 0; i < (is_2d ? 2 : 3); i++)
  {
    if (tmin_s[i] > t_enter)
    {
      t_enter = tmin_s[i];
      axis = i;
    }
  }

  float t_exit = fminf(tmax_s[0], tmax_s[1]);
  if (!is_2d)
    t_exit = fminf(t_exit, tmax_s[2]);

  if (t_enter > t_exit || t_enter < 0.0f || t_enter > 1.0f)
    return false;

  if (factor)
    *factor = t_enter;

  if (coll_axis)
    *coll_axis = axis;

  return true;
}

bool resolve_walkthrough(entity a, entity b);
void compute_collisions()
{
  float *radii = aabb_component->streams->radius;
  struct vec4_st *extents = aabb_component->streams->extent;
  struct vec4_st *physix_positions = position_component->stream->position;
  struct vec4_st *physix_previous_positions =
      position_component->stream->prev_position;

  if (!physix_previous_positions)
    physix_previous_positions = physix_positions;

  for (uint32_t aabb_i = 0; aabb_i < aabb_component->set.count; ++aabb_i)
  {
    entity entity_a = aabb_component->set.dense[aabb_i];
    uint32_t pos_i;

    if (!component_get_dense_id((struct generic_component *)position_component,
                                entity_a, &pos_i))
      continue;

    float32x4_t min_a;
    float32x4_t max_a;
    compute_swept_aabb_box(
        &physix_positions[pos_i],
        physix_previous_positions ? &physix_previous_positions[pos_i] : NULL,
        &extents[aabb_i], &min_a, &max_a);

    for (uint32_t aabb_j = aabb_i + 1; aabb_j < aabb_component->set.count;
         ++aabb_j)
    {
      entity entity_b = aabb_component->set.dense[aabb_j];
      uint32_t pos_j;

      if (!component_get_dense_id((struct generic_component *)position_component,
                                  entity_b, &pos_j))
        continue;

      if (!radii_collide(physix_previous_positions + pos_i,
                         physix_positions + pos_i,
                         physix_previous_positions + pos_j,
                         physix_positions + pos_j, radii[pos_i], radii[pos_j]))
        continue;

      float32x4_t min_b;
      float32x4_t max_b;

      compute_swept_aabb_box(
          &physix_positions[pos_j],
          physix_previous_positions ? &physix_previous_positions[pos_j] : NULL,
          &extents[aabb_j], &min_b, &max_b);

      if (check_aabb_overlap(min_a, max_a, min_b, max_b))
      {
        // resolve_walkthrough(entity_a, entity_b);
        resolve_collision(entity_a, entity_b);
      }
    }
  }
}

#if 0
static inline bool may_collide(struct vec4_st *prev_pos_a, struct vec4_st *prev_pos_b, struct vec4_st *pos_b, float radii_a, float radii_b)
{
  if (!radii_collide(prev_pos_a, prev_pos_b, pos_b, radii_a, radii_b))
    return false;
}

void compute_minkowski_collisions()
{
  float *radii = aabb_component->streams->radius;
  struct vec4_st *extents = aabb_component->streams->extent;
  struct vec4_st *physix_positions = position_component->stream->position;
  struct vec4_st *physix_previous_positions = position_component->stream->prev_position;

  if (!physix_previous_positions)
    physix_previous_positions = physix_positions;

  for (uint32_t aabb_i = 0; aabb_i < aabb_component->set.count; ++aabb_i)
  {
    entity entity_a = aabb_component->set.dense[aabb_i];
    uint32_t pos_i;

    if (!component_get_dense_id((struct generic_component *)position_component, entity_a, &pos_i))
      continue;

#if 0
    float32x4_t min_a;
    float32x4_t max_a;
    compute_swept_aabb_box(&physix_positions[pos_i],
                           physix_previous_positions ? &physix_previous_positions[pos_i] : NULL,
                           &extents[aabb_i], &min_a, &max_a);
#endif

    for (uint32_t aabb_j = aabb_i + 1; aabb_j < aabb_component->set.count;
         ++aabb_j)
    {
      entity entity_b = aabb_component->set.dense[aabb_j];
      uint32_t pos_j;

      if (!component_get_dense_id((struct generic_component *)position_component, entity_b, &pos_j))
        continue;

      // auto relative_vel = compute_relative_velocity(physix_previous_positions[pos_i], physix_positions[pos_j], physix_previous_positions[pos_j], physix_positions[pos_j], TIMESTEP);

      if (!may_collide(physix_previous_positions + pos_i, physix_previous_positions + pos_j, physix_positions + pos_j, radii[pos_i], radii[pos_j]))
        continue;

#if 0
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
#endif
      {
        // ToDo:  Use Minkowski sum
        resolve_collision(entity_a, entity_b);
      }
    }
  }
}
#endif

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

void update_position(entity e, float *pos, float *prev_pos, float fac, int coll_axis)
{
  struct vec4_st *vel = get_velocity(e);

  float32x4_t current_p = vld1q_f32((float *)pos);
  float32x4_t previous_p = vld1q_f32((float *)prev_pos);

  if (has_component(e, (struct generic_component *)velocity_component) && vel)
  {
    // Move entity A back by collision factor
    float32x4_t ds = vmulq_n_f32(vsubq_f32(current_p, previous_p), fac);
    current_p = vaddq_f32(previous_p, ds);

    // Stop velocity in collision axis
    if (coll_axis == 0)
      vel->x = 0;
    if (coll_axis == 1)
      vel->y = 0;
    if (coll_axis == 2)
      vel->z = 0;

    current_p = vmlaq_n_f32(current_p, vld1q_f32(vel), TIMESTEP * (1 - fac));
    vst1q_f32(pos, current_p);
  }
}

bool resolve_walkthrough(entity a, entity b)
{
  struct vec4_st *extents = aabb_component->streams->extent;
  struct vec4_st *physix_positions = position_component->stream->position;
  struct vec4_st *physix_previous_positions = position_component->stream->prev_position;

  uint32_t pos_a, pos_b, aabb_a, aabb_b;

  if (!component_get_dense_id((struct generic_component *)position_component, a, &pos_a))
    return false;
  if (!component_get_dense_id((struct generic_component *)position_component, b, &pos_b))
    return false;

  if (!component_get_dense_id((struct generic_component *)aabb_component, a, &aabb_a))
    return false;
  if (!component_get_dense_id((struct generic_component *)aabb_component, b, &aabb_b))
    return false;

  int coll_axis;
  float fac;

  // Check for collision
  if (!ray_box_collision(&physix_previous_positions[pos_a],
                         &physix_previous_positions[pos_b],
                         &physix_positions[pos_a],
                         &physix_positions[pos_b],
                         &extents[aabb_a],
                         &extents[aabb_b],
                         &fac,
                         &coll_axis))
  {
    return false;
  }

  if (fac > 1.0f || fac < 0.0f)
    return false;

  // Adjust positions based on collision
  float safe_fac = fac > GREY_ZERO ? fac - GREY_ZERO : 0.0f;

  // Get velocities

  update_position(a, (float *)&physix_positions[pos_a], (float *)&physix_previous_positions[pos_a], safe_fac, coll_axis);
  update_position(b, (float *)&physix_positions[pos_b], (float *)&physix_previous_positions[pos_b], safe_fac, coll_axis);

  return true;
}
bool walk_through_resolution(event *e)
{
  // return false;
  printf("______________%s\n", __FUNCTION__);

  if (e->type != COLLISION_EVENT)
    return false;

  collision_data *data = e->info;

  return resolve_walkthrough(data->a, data->b);
}

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
