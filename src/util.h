// util.h: utilities that don't fit in any specific file
#ifndef UTIL_H
#define UTIL_H
#include <math.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include "types.h"

void SeedRNG(void);

u32 RandomU32(u32 min, u32 max);

i32 RandomI32(i32 min, i32 max);

f64 RandomF64(f64 min, f64 max);

bool CoinFlip(f64 chance);

bool IsPrime(u32 x);

// Shuffle array of u32s from i to j (i and j can wrap around)
void ShuffleArrayU32(u32 *array, u32 n_elements, u32 i, u32 j);

// Reverse array of u32s from i to j (i and j can wrap around)
void ReverseArrayU32(u32 *array, u32 n_elements, u32 i, u32 j);

i32 RoundNearest(f64 x);

#endif // UTIL_H
