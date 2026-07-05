#include "util.h"

void SeedRNG(u32 seed) {
    srand(seed);
}

i32 RandomInt(i32 min, i32 max) {
    assert(max >= min);
    if (min == max) return min;
    return min + rand()%(max - min + 1);
}

f64 RandomFloat(f64 min, f64 max) {
    assert(max >= min);
    return min + ((f64)rand()/(f64)RAND_MAX)*(max - min);
}

bool CoinFlip(f64 chance) {
    assert(0.0 <= chance && chance <= 1.0);
    if (chance == 0.0) return false;
    return RandomFloat(0.0, 1.0) <= chance;
}

// TODO: this algorithm is slow, but should be fine for our purposes
bool IsPrime(u32 x) {
    if (x == 2) {
        return true;
    }
    if (x <= 1 || x%2 == 0) {
        return false;
    }

    for (u32 i = 3; i*i <= x; i += 2) {
        if (x%i == 0) {
            return false;
        }
    }

    return true;
}

i32 RoundNearest(f64 x) {
    return (i32)(x + 0.5);
}
