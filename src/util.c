#include "util.h"

void SeedRNG(void) {
    srand(time(NULL));
}

i32 RandomI32(i32 min, i32 max) {
    assert(max > min);
    return min + rand()%(max - min + 1);
}

f64 RandomF64(f64 min, f64 max) {
    return min + (f64)rand()/(f64)RAND_MAX*(max - min);
}

bool CoinFlip(f64 chance) {
    return RandomF64(0.0, 1.0) < chance;
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

// Fihser-Yates shuffling algorithm
void ShuffleArray(u32 *array, u32 n_elements) {
    if (n_elements <= 1) {
        return;
    }
    for (u32 i = 0; i <= n_elements - 2; i++) {
        u32 j = RandomI32(i, n_elements - 1);
        u32 temp = array[i];
        array[i] = array[j];
        array[j] = temp;
    }
}

void ReverseArray(u32 *array, u32 n_elements) {
    u32 *p1 = array;
    u32 *p2 = array + n_elements - 1;
    while (p1 < p2) {
        u32 temp = *p1;
        *p1 = *p2;
        *p2 = temp;
        p1++;
        p2--;
    }
}

i32 RoundNearest(f64 x) {
    return (i32)(x + 0.5);
}
