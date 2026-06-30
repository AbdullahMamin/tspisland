// util.h: utilities that don't fit in any specific file
#ifndef UTIL_H
#define UTIL_H
#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include "types.h"

void SeedRNG(void);

i32 RandomI32(i32 min, i32 max);

f64 RandomF64(f64 min, f64 max);

bool CoinFlip(f64 chance);

bool IsPrime(u32 x);

// Shuffle array of u32s
void ShuffleArray(u32 *array, u32 n_elements);

// Reverse array of u32s
void ReverseArray(u32 *array, u32 n_elements);

i32 RoundNearest(f64 x);

#endif // UTIL_H
