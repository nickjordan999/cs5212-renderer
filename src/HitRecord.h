#ifndef HITRECORD_H
#define HITRECORD_H

#include "vec.h"

struct HitRecord {
    float t;           // Parameter along the ray where intersection occurs
    vec3 point;        // World space intersection point
    vec3 normal;       // Surface normal at intersection point
    vec3 material;     // Material color
};

#endif // HITRECORD_H
