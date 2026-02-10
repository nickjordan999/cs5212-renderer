#ifndef HITRECORD_H
#define HITRECORD_H

#include "vec.h"
#include "Material.h"

struct HitRecord {
    float t;           // Parameter along the ray where intersection occurs
    vec3 point;        // World space intersection point
    vec3 normal;       // Surface normal at intersection point
    Material material; // Material properties
};

#endif // HITRECORD_H
