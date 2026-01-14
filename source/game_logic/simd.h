#ifndef SIMD_H
#define SIMD_H

#if defined(__ARM_NEON__)
#include <arm_neon.h>
#include <assert.h>
#include <game_logic.h>
#include <zot.h>

#if !(__ARM_ARCH >= 8 && defined(__ARM_FEATURE_DIRECTED_ROUNDING))
static inline float vaddvq_f32(float32x4_t v) {
  float32x2_t v2 = vadd_f32(vget_low_f32(v), vget_high_f32(v));
  v2 = vpadd_f32(v2, v2);
  return vget_lane_f32(v2, 0);
}

static inline float32x2_t vdiv_f32(float32x2_t dividend, float32x2_t divisor) {
#if defined(DEBUG) || !defined(NDEBUG)
  if (vget_lane_f32(divisor, 0) <= GREY_ZERO || vget_lane_f32(divisor, 1) <= GREY_ZERO) {
    LOG_ERROR("SIMD Division by Zero detected in vdiv_f32!");
    assert(0);
  }
#endif

  uint32x2_t is_not_zero_mask = vcgt_f32(divisor, vdup_n_f32(GREY_ZERO));

  float32x2_t safe_divisor = vbsl_f32(is_not_zero_mask, divisor, vdup_n_f32(1.f));

  auto reciprocal = vrecpe_f32(safe_divisor);
  reciprocal = vmul_f32(vrecps_f32(safe_divisor, reciprocal), reciprocal);
  reciprocal = vmul_f32(vrecps_f32(safe_divisor, reciprocal), reciprocal);

  return vmul_f32(dividend, reciprocal);
}

#endif

#elif defined(__AVX__) || defined(__SSE__)
#include "NEON_2_SSE.h"
#endif

#endif