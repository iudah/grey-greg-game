#ifndef SIMD_H
#define SIMD_H

#if defined(__ARM_NEON__)
#include <arm_neon.h>

#if !(__ARM_ARCH >= 8 && defined(__ARM_FEATURE_DIRECTED_ROUNDING))
static inline float vaddvq_f32(float32x4_t v) {
  float32x2_t v2 = vadd_f32(vget_low_f32(v), vget_high_f32(v));
  v2 = vpadd_f32(v2, v2);
  return vget_lane_f32(v2, 0);
}

#endif

#elif defined(__AVX__) || defined(__SSE__)
#include "NEON_2_SSE.h"
#endif

#endif