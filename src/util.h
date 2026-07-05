// util.h: utilities that don't fit in any specific file
#ifndef UTIL_H
#define UTIL_H
#include <math.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include "types.h"

#define RANDOM_SEED (time(NULL))

// TODO: Uniformly distributed random function

void SeedRNG(u32 seed);

i32 RandomInt(i32 min, i32 max);

f64 RandomFloat(f64 min, f64 max);

bool CoinFlip(f64 chance);

bool IsPrime(u32 x);

i32 RoundNearest(f64 x);

#endif // UTIL_H
