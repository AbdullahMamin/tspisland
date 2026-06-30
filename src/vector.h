// vector.h: vector math
#ifndef VECTOR_H
#define VECTOR_H
#include <math.h>
#include "types.h"

typedef struct {
    f64 x, y;
} vec2;

vec2 Vec2(f64 x, f64 y);
f64 Vec2Distance(vec2 v1, vec2 v2);

#endif // VECTOR_H
