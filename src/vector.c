#include "vector.h"

vec2 Vec2(f64 x, f64 y) {
    return (vec2){x, y};
}

f64 Vec2Distance(vec2 v1, vec2 v2) {
    f64 dx = v2.x - v1.x;
    f64 dy = v2.y - v1.y;
    return sqrt(dx*dx + dy*dy);
}
