#include "physics_system.h"

#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <zot.h>

#include "collision_component.h"
#include "collision_spatial_hash.h"
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

typedef struct {
  entity a, b;
  // float collision_time_factor;
} collision_data;

float distance(struct vec4_st *npc_pos, struct vec4_st *player_pos, bool flee,
               float32x4_t *diff_ptr) {
  auto n_pos = vld1q_f32((float *)npc_pos);
  auto p_pos = vld1q_f32((float *)player_pos);

  auto diff = flee ? vsubq_f32(n_pos, p_pos) : vsubq_f32(p_pos, n_pos);
  diff = vsetq_lane_f32(0, diff, 3);  // zero 'w'

  if (diff_ptr) {
    memcpy(diff_ptr, &diff, sizeof(diff));
  }

  auto diff2 = vmulq_f32(diff, diff);
  auto sum = vadd_f32(vget_high_f32(diff2), vget_low_f32(diff2));
  sum = vpadd_f32(sum, sum);

  return sqrtf(vget_lane_f32(sum, 0));
}

bool collision_overlap(struct vec4_st *a_pos, struct vec4_st *a_ext, struct vec4_st *b_pos,
                       struct vec4_st *b_ext) {
  auto a_pos_simd = vld1q_f32((float *)a_pos);
  auto b_pos_simd = vld1q_f32((float *)b_pos);
  auto a_ext_simd = vld1q_f32((float *)a_ext);
  auto b_ext_simd = vld1q_f32((float *)b_ext);

  auto pos_diff = vabsq_f32(vsubq_f32(a_pos_simd, b_pos_simd));
  auto ext_sum = vaddq_f32(a_ext_simd, b_ext_simd);

  auto l_eq = vcleq_f32(pos_diff, ext_sum);

  return (vgetq_lane_s32(l_eq, 0) & vgetq_lane_s32(l_eq, 1) & vgetq_lane_s32(l_eq, 2)) != 0;
}

void euler_method() {
  for (uint32_t i = 0; i < velocity_component->set.count; ++i) {
    entity entity = velocity_component->set.dense[i];
    auto position = get_position(entity);
    auto velocity = get_velocity(entity);

    if (!position || !velocity) continue;

    auto t = vdupq_n_f32(TIMESTEP);
    auto v = vld1q_f32((void *)velocity);

    float default_mass = 1.f;
    float *mass = get_mass(entity);
    if (!mass || *mass < GREY_ZERO) mass = &default_mass;

    auto zero_f = (struct vec4_st){0, 0, 0, 0};
    struct vec4_st *force = get_force(entity);
    if (!force) force = &zero_f;

    auto f = vld1q_f32((float *)force);
    auto a = vmulq_f32(f, vdupq_n_f32(1. / *mass));
    v = vmlaq_f32(v, a, t);

    auto p = vld1q_f32((void *)position);
    p = vmlaq_f32(p, v, t);

    vst1q_f32((void *)velocity, v);
    vst1q_f32((void *)position, p);
  }
}

#if 0
void verlet_integration_method()
{
  struct vec4_st *prev_pos_arr = position_component->streams->prev_position;
  struct vec4_st *pos_arr = position_component->streams->position;
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

void physics_system_update(game_logic *logic, float delta_time) {
  euler_method();
  update_spatial_partition();
  compute_collisions(logic);
}

void compute_swept_collision_box(struct vec4_st *curr_pos, struct vec4_st *prev_pos,
                                 struct vec4_st *extent, float32x4_t *out_min,
                                 float32x4_t *out_max) {
  if (!prev_pos) {
    prev_pos = curr_pos;
  }

  auto prev = vld1q_f32((float *)prev_pos);
  auto curr = vld1q_f32((float *)curr_pos);

  auto half_extent = vld1q_f32((float *)extent);

  // Ensure W component is zero (unused)
  vsetq_lane_f32(0.f, half_extent, 3);

  auto swept_min = vminq_f32(vsubq_f32(prev, half_extent), vsubq_f32(curr, half_extent));

  auto swept_max = vmaxq_f32(vaddq_f32(prev, half_extent), vaddq_f32(curr, half_extent));

  if (out_min) {
    memcpy(out_min, &swept_min, sizeof(swept_min));
  }
  if (out_max) {
    memcpy(out_max, &swept_max, sizeof(swept_max));
  }
}

bool check_collision_overlap(float32x4_t min_a, float32x4_t max_a, float32x4_t min_b,
                             float32x4_t max_b) {
  uint32x4_t le_max = vcleq_f32(vsubq_f32(min_a, max_b), vdupq_n_f32(GREY_ZERO));
  uint32x4_t ge_min = vcleq_f32(vsubq_f32(min_b, max_a), vdupq_n_f32(GREY_ZERO));
  uint32x4_t overlap_mask = vandq_u32(le_max, ge_min);

  uint32_t result[4];
  vst1q_u32(result, overlap_mask);

  // Only check x, y, z (ignore w)
  return result[0] && result[1] && result[2];
}

void event_enqueue_collision(game_logic *logic, entity entity_i, entity entity_j) {
  collision_data *collision = zmalloc(sizeof(*collision));

  collision->a = entity_i;
  collision->b = entity_j;

  event_trigger(game_logic_get_event_system(logic), collision, COLLISION_EVENT);
}

void resolve_collision(game_logic *logic, entity entity_i, entity entity_j) {
  event_enqueue_collision(logic, entity_i, entity_j);

  // LOG("Collision between entity %d and %d", entity_i.id, entity_j.id);

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

static inline bool radii_collide(struct vec4_st *prev_pos_a, struct vec4_st *pos_a,
                                 struct vec4_st *prev_pos_b, struct vec4_st *pos_b, float radii_a,
                                 float radii_b) {
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
  if (v_len_sq > GREY_ZERO) {
    t = dot / v_len_sq;
  }

  if (t < 0.0f) {
    t = 0;
  }
  if (t > 1.0f) {
    t = 1;
  }
  auto closest = vmlaq_n_f32(pb, v_rel, t);

  auto dist_vec = vsubq_f32(pa, closest);
  float dist_sq = vaddvq_f32(vmulq_f32(dist_vec, dist_vec));
  float r_sum = radii_a + radii_b;

  return dist_sq <= (r_sum * r_sum);
}

static inline float magnitude(float32x4_t v) {
  auto v2 = vmulq_f32(v, v);
  return sqrtf(vaddvq_f32(v2));
}

bool ray_box_collision(struct vec4_st *prev_pos_a, struct vec4_st *prev_pos_b,
                       struct vec4_st *pos_a, struct vec4_st *pos_b, struct vec4_st *extents_a,
                       struct vec4_st *extents_b, float *factor, int *coll_axis) {
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

  if (vgetq_lane_u32(rel_motion_mask, 0) != 0 && vgetq_lane_u32(rel_motion_mask, 1) != 0 &&
      (grey_is_2d() || vgetq_lane_u32(rel_motion_mask, 2) != 0))
    return false;

  float32x4_t extent_sum = vaddq_f32(vld1q_f32((float *)extents_a), vld1q_f32((float *)extents_b));

  // Zero w
  pa0 = vsetq_lane_f32(0, pa0, 3);
  pb0 = vsetq_lane_f32(0, pb0, 3);
  extent_sum = vsetq_lane_f32(0, extent_sum, 3);

  float32x4_t left = vsubq_f32(pa0, extent_sum);
  float32x4_t right = vaddq_f32(pa0, extent_sum);

  // ray dir
  float32x4_t dir = vsubq_f32(vsubq_f32(pb1, pb0), vsubq_f32(pa1, pa0));

  float pb0_arr[4];
  float dir_arr[4];
  float left_arr[4];
  float right_arr[4];
  vst1q_f32(dir_arr, dir);
  vst1q_f32(left_arr, left);
  vst1q_f32(right_arr, right);
  vst1q_f32(pb0_arr, pb0);

  float tmin_s[4] = {-INFINITY, -INFINITY, -INFINITY, 0};
  float tmax_s[4] = {INFINITY, INFINITY, INFINITY, 0};

  for (uint8_t i = 0; i < (grey_is_2d() ? 2 : 3); ++i) {
    if (fabsf(dir_arr[i]) < GREY_ZERO) {
      if (pb0_arr[i] < left_arr[i] || pb0_arr[i] > right_arr[i]) return false;
    } else {
      float inv = 1.0f / dir_arr[i];
      float t0 = (left_arr[i] - pb0_arr[i]) * inv;
      float t1 = (right_arr[i] - pb0_arr[i]) * inv;
      tmin_s[i] = fminf(t0, t1);
      tmax_s[i] = fmaxf(t0, t1);
    }
  }

  float t_enter = -INFINITY;
  int axis = 0;
  for (int i = 0; i < (grey_is_2d() ? 2 : 3); i++) {
    if (tmin_s[i] > t_enter) {
      t_enter = tmin_s[i];
      axis = i;
    }
  }

  float t_exit = fminf(tmax_s[0], tmax_s[1]);
  if (!grey_is_2d()) t_exit = fminf(t_exit, tmax_s[2]);

  if (t_enter > t_exit || t_exit < 0.0f || t_enter > 1.0f) return false;

  if (t_enter < 0.0f) t_enter = 0.0f;

  if (factor) *factor = t_enter;

  if (coll_axis) *coll_axis = axis;

  return true;
}

void compute_collisions(game_logic *logic) {
  // float *radii = collision_component->streams->collision_radius;
  // struct vec4_st *extents = collision_component->streams->collision_extent;
  // struct vec4_st *physix_positions = position_component->streams->position;
  // struct vec4_st *physix_previous_positions = position_component->streams->previous_position;

  // if (!physix_previous_positions) physix_previous_positions = physix_positions;

  for (uint32_t collision_i = 0; collision_i < collision_component->set.count; ++collision_i) {
    entity entity_a = get_entity(collision_component, collision_i);
    // uint32_t pos_i;

    // if (!component_get_dense_id((struct generic_component *)position_component, entity_a,
    // &pos_i))
    //   continue;

    float *radius_a = get_collision_radius(entity_a);
    struct vec4_st *extent_a = get_collision_extent(entity_a);
    struct vec4_st *position_a = get_position(entity_a);
    struct vec4_st *prev_position_a = get_previous_position(entity_a);
    if (!prev_position_a) prev_position_a = position_a;
    if (!radius_a || !extent_a || !position_a || !radius_a) continue;

    float32x4_t min_a;
    float32x4_t max_a;
    compute_swept_collision_box(position_a, prev_position_a, extent_a, &min_a, &max_a);

    for (uint32_t collision_j = collision_i + 1; collision_j < collision_component->set.count;
         ++collision_j) {
      entity entity_b = get_entity(collision_component, collision_j);

      if (!belong_to_same_collision_layer(entity_a, entity_b)) continue;

      float *radius_b = get_collision_radius(entity_b);
      struct vec4_st *extent_b = get_collision_extent(entity_b);
      struct vec4_st *position_b = get_position(entity_b);
      struct vec4_st *prev_position_b = get_previous_position(entity_b);
      if (!prev_position_b) prev_position_b = position_b;
      if (!extent_b || !position_b || !radius_b) continue;

      if (!radii_collide(prev_position_a, position_a, prev_position_b, position_b, *radius_a,
                         *radius_b))
        continue;

      float32x4_t min_b;
      float32x4_t max_b;

      compute_swept_collision_box(position_b, prev_position_b, extent_b, &min_b, &max_b);

      if (check_collision_overlap(min_a, max_a, min_b, max_b)) {
        resolve_collision(logic, entity_a, entity_b);
      }
    }
  }
}

void update_position(entity e, float *pos, float *prev_pos, float fac, int coll_axis) {
  struct vec4_st *vel = get_velocity(e);

  float32x4_t current_p = vld1q_f32((float *)pos);
  float32x4_t previous_p = vld1q_f32((float *)prev_pos);

  if (has_component(e, (struct generic_component *)velocity_component) && vel) {
    // Move entity A back by collision factor
    float32x4_t ds = vmulq_n_f32(vsubq_f32(current_p, previous_p), fac);
    current_p = vaddq_f32(previous_p, ds);

    // Stop velocity in collision axis
    if (coll_axis == 0) vel->x = 0;
    if (coll_axis == 1) vel->y = 0;
    if (coll_axis == 2) vel->z = 0;

    current_p = vmlaq_n_f32(current_p, vld1q_f32((float *)vel), TIMESTEP * (1 - fac));
    vst1q_f32(pos, current_p);
  }
}

bool resolve_walkthrough(entity a, entity b) {
  struct vec4_st *prev_pos_a = get_previous_position(a);
  struct vec4_st *prev_pos_b = get_previous_position(b);
  struct vec4_st *pos_a = get_position(a);
  struct vec4_st *pos_b = get_position(b);
  struct vec4_st *collision_extent_a = get_collision_extent(a);
  struct vec4_st *collision_extent_b = get_collision_extent(b);

  if (!prev_pos_a || !prev_pos_b || !pos_a || !pos_b || !collision_extent_a || !collision_extent_b)
    return false;

  int coll_axis;
  float fac;

  // Check for collision
  if (!ray_box_collision(prev_pos_a, prev_pos_b, pos_a, pos_b, collision_extent_a,
                         collision_extent_b, &fac, &coll_axis)) {
    return false;
  }

  if (fac > 1.0f || fac < 0.0f) return false;

  // Adjust positions based on collision
  float safe_fac = fac > GREY_COLLISION_GAP ? fac - GREY_COLLISION_GAP : 0.0f;

  update_position(a, (float *)pos_a, (float *)prev_pos_a, safe_fac, coll_axis);
  update_position(b, (float *)pos_b, (float *)prev_pos_b, safe_fac, coll_axis);

  return true;
}

bool walk_through_resolution(event *e) {
  // printf("______________%s\n", __FUNCTION__);

  if (e->type != COLLISION_EVENT) return false;

  collision_data *data = e->info;

  return resolve_walkthrough(data->a, data->b);
}

struct vec4_st *get_next_patrol_point(entity e) { return get_waypoint(e); }

bool advance_patrol_index(entity e) {
  auto waypoint = get_waypoint(e);

  if (!waypoint) return false;

  // TODO: Use angle of view to randomly choose waypoint

  waypoint->x++;
  waypoint->y++;
  waypoint->z++;

  return true;
}
