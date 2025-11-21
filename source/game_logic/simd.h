#ifndef SIMD_H
#define SIMD_H

#if defined(__ARM_NEON__)
#include <arm_neon.h>
#elif defined(__AVX__) || defined(__SSE__)
#include "NEON_2_SSE.h"
#endif

#endif