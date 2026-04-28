#ifndef HITRECORD_H
#define HITRECORD_H

#include "vec.h"
#include "Material.h"

struct HitRecord {
    float t;                    // Parameter along the ray where intersection occurs
    vec3 point;                 // World space intersection point
    vec3 normal;                // Surface normal at intersection point (geometric, NOT auto-flipped)
    Material material;          // Material properties
    bool is_front_face = true;  // True if the ray hits the side of the surface where its geometric
                                // normal points outward (i.e., ray.direction.dot(normal) < 0).
                                // Consumers needing a normal oriented against the ray should flip
                                // when is_front_face is false.
};

#endif // HITRECORD_H
