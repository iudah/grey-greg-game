#ifndef ARM_NEON_ITUNU
#define ARM_NEON_ITUNU

#ifdef __arm__
#include <arm_neon.h>
#else
#include "NEON_2_SSE.h"
#endif

#if defined __arm__ && defined __ARM_FP && !defined __LITTLE_ENDIAN__
#error Sorry but I am trying to finish this project and currently only support little endian arm because that is what I use.
#endif

#define __ai static __inline__ __attribute__((__always_inline__, __nodebug__))

__ai __attribute__((target("neon"))) float32x4_t
vdivq_f32(float32x4_t dividend, float32x4_t divisor)
{
  /*determine an initial estimate of reciprocal of divisor.*/
  auto initial_reciprocal = vrecpeq_f32(divisor);
  auto correction_factor = vrecpsq_f32(divisor, initial_reciprocal);
  initial_reciprocal = vmulq_f32(initial_reciprocal, correction_factor);
  correction_factor = vrecpsq_f32(divisor, initial_reciprocal);
  initial_reciprocal = vmulq_f32(initial_reciprocal, correction_factor);

  return vmulq_f32(dividend, initial_reciprocal);
}

__ai __attribute__((target("neon"))) float vaddvq_f32(float32x4_t a)
{
  auto sum = vadd_f32(vget_high_f32(a), vget_low_f32(a));
  sum = vpadd_f32(sum, sum);

  return vget_lane_f32(sum, 0);
}

__ai __attribute__((target("neon"))) float vminvq_f32(float32x4_t a)
{
  auto min = vmin_f32(vget_high_f32(a), vget_low_f32(a));
  min = vpmin_f32(min, min);
  return vget_lane_f32(min, 0);
}

__ai __attribute__((target("neon"))) float vmaxvq_f32(float32x4_t a)
{
  auto max = vmax_f32(vget_high_f32(a), vget_low_f32(a));
  max = vpmax_f32(max, max);
  return vget_lane_f32(max, 0);
}

__ai __attribute__((target("neon"))) float32x4_t SIMD_round(float32x4_t a)
{
  // https://stackoverflow.com/a/69770515
  auto a_as_int = vcvtq_n_s32_f32(a, 1);
  auto arithmetic_shift_right =
      vsraq_n_s32(a_as_int, a_as_int, 31); // account for negative rounding
  auto floor_round = vrshrq_n_s32(arithmetic_shift_right, 1);
  return vcvtq_f32_s32(floor_round);
}

#define LN2_MUL_INV \
  1.442695040888963407359924681001892137426645954152985934135449406931109219181185079885526622893506344f
#define LN2 \
  0.6931471805599453094172321214581765680755001343602552541206800094933936219696947156058633269964186875f

__ai __attribute__((target("neon"))) float32x4_t SIMD_exp(float32x4_t a)
{
  auto a_over_ln2 = vmulq_n_f32(a, LN2_MUL_INV);

  auto n = SIMD_round(a_over_ln2);

  auto r = vsubq_f32(a, vmulq_n_f32(n, LN2));
  auto r2 = vmulq_f32(r, r);
  auto r3 = vmulq_f32(r2, r);
  auto r4 = vmulq_f32(r3, r);
  auto r5 = vmulq_f32(r4, r);
  auto r6 = vmulq_f32(r5, r);

  auto exp_r_poly = vaddq_f32(
      vdupq_n_f32(1.f),
      vaddq_f32(
          r,
          vaddq_f32(
              vmulq_n_f32(r2, 0.5f),
              vaddq_f32(
                  vmulq_n_f32(
                      r3, 0.16666666666666666666666666666666666666666666667f),
                  vaddq_f32(
                      vmulq_n_f32(r4,
                                  0.0416666666666666666666666666666666666667f),
                      vaddq_f32(
                          vmulq_n_f32(
                              r5,
                              0.0083333333333333333333333333333333333333333333333333333333f),
                          vmulq_n_f32(
                              r6,
                              0.001388888888888888888888888888888888888888888889f)))))));

  // Convert n (float) to integer for bit-level manipulation
  auto n_int = vcvtq_s32_f32(n);

  // Compute 2^n by constructing the floating-point exponent:
  // In IEEE 754 single precision, the exponent bias is 127.
  int32x4_t exp_int = vshlq_n_s32(vaddq_s32(n_int, vdupq_n_s32(127)), 23);
  float32x4_t pow2n = vreinterpretq_f32_s32(exp_int);

  // exp(x) ≈ 2^n * poly(r)
  return vmulq_f32(pow2n, exp_r_poly);
}

__ai __attribute__((target("neon"))) float32x4_t SIMD_log(float32x4_t a)
{

  // Reinterpret input to extract bits
  uint32x4_t x_bits = vreinterpretq_u32_f32(a);

  // Extract exponent and compute k
  uint32x4_t exponent_bits = vshrq_n_u32(x_bits, 23);
  exponent_bits = vandq_u32(exponent_bits, vdupq_n_u32(0xFF));
  int32x4_t k_i =
      vsubq_s32(vreinterpretq_s32_u32(exponent_bits), vdupq_n_s32(127));
  float32x4_t k = vcvtq_f32_s32(k_i);

  // Reconstruct mantissa m in [1, 2)
  uint32x4_t m_bits = vandq_u32(x_bits, vdupq_n_u32(0x007FFFFF));
  m_bits = vorrq_u32(m_bits, vdupq_n_u32(0x3F800000));
  float32x4_t m = vreinterpretq_f32_u32(m_bits);

  // Compute f = m - 1.0f and z = f / (m + 1.0f)
  float32x4_t f = vsubq_f32(m, vdupq_n_f32(1.0f));
  float32x4_t denominator = vaddq_f32(m, vdupq_n_f32(1.0f));

  // Compute reciprocal of denominator using Newton-Raphson
  float32x4_t z = vdivq_f32(f, denominator);

  // Compute polynomial approximation
  float32x4_t w = vmulq_f32(z, z);
  const float32x4_t R0 = vdupq_n_f32(7.0376836292E-2f);
  const float32x4_t R1 = vdupq_n_f32(-1.1514610310E-1f);
  const float32x4_t R2 = vdupq_n_f32(1.1676998740E-1f);
  const float32x4_t R3 = vdupq_n_f32(-1.2420140846E-1f);

  float32x4_t poly = vmlaq_f32(R2, w, R3);
  poly = vmlaq_f32(R1, w, poly);
  poly = vmlaq_f32(R0, w, poly);

  float32x4_t R = vmlaq_f32(z, vmulq_f32(z, w), poly);
  float32x4_t log_m = vaddq_f32(R, R); // 2 * R

  // Combine with exponent contribution
  const float32x4_t ln2 = vdupq_n_f32(0.69314718056f);
  float32x4_t result = vmlaq_f32(log_m, k, ln2);

  return result;
}

#endif