#include "util.h"

void SeedRNG(void) {
    srand(time(NULL));
}

u32 RandomU32(u32 min, u32 max) {
    assert(max > min);
    return min + rand()%(max - min + 1);
}

i32 RandomI32(i32 min, i32 max) {
    assert(max > min);
    return min + rand()%(max - min + 1);
}

f64 RandomF64(f64 min, f64 max) {
    assert(max > min);
    return min + (f64)rand()/(f64)RAND_MAX*(max - min);
}

bool CoinFlip(f64 chance) {
    assert(fabs(chance) <= 1.0);
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
void ShuffleArrayU32(u32 *array, u32 n_elements, u32 i, u32 j) {
    assert(array && i < n_elements && j < n_elements);

    u32 length = (i <= j) ? (j - i + 1) : (n_elements - j + i + 1);
    if (length <= 1) {
        return;
    }

    for (u32 a = 0; a <= length - 2; a++) {
        u32 b = RandomU32(a, length - 1);
        u32 idx1 = (i + a)%n_elements;
        u32 idx2 = (i + b)%n_elements;
        u32 temp = array[idx1];
        array[idx1] = array[idx2];
        array[idx2] = temp;
    }
}

void ReverseArrayU32(u32 *array, u32 n_elements, u32 i, u32 j) {
    assert(array && i < n_elements && j < n_elements);

    u32 length = (i <= j) ? (j - i + 1) : (n_elements - j + i + 1);
    if (length <= 1) {
        return;
    }

    for (u32 a = 0; a < length/2; a++) {
        u32 temp = array[i];
        array[i] = array[j];
        array[j] = temp;
        i = (i + 1)%n_elements;
        if (j == 0) {
            j = n_elements - 1;
        } else {
            j--;
        }
    }
}

i32 RoundNearest(f64 x) {
    return (i32)(x + 0.5);
}
