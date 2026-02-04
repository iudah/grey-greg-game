#include "irand.h"

#include <entropy.h>
#include <pcg_variants.h>
#include <stdint.h>
#include <stdlib.h>

#define RAND_SEED

uint64_t irand() {
  static bool seeded = false;
  static pcg32_random_t rng;

  if (!seeded) {
    seeded = true;
#ifdef RAND_SEED
    uint64_t seeds[2];
    entropy_getbytes((void *)seeds, sizeof(seeds));
    pcg32_srandom_r(&rng, seeds[0], seeds[1]);
#else
    pcg32_srandom_r(&rng, 42u, 54u);

#endif
  }

  uint64_t high = pcg32_random_r(&rng);
  uint64_t low = pcg32_random_r(&rng);

  return (high << 32) | low;
}