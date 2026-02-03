#ifndef RAY_H
#define RAY_H

#include "vec.h"

class Ray {
public:
    // Constructor: creates a ray from an origin point in a given direction
    Ray(const vec3& origin, const vec3& direction)
        : origin(origin), direction(direction.normalized()) {}

    // Get the origin (base) of the ray
    const vec3& getOrigin() const {
        return origin;
    }

    // Get the normalized direction of the ray
    const vec3& getDirection() const {
        return direction;
    }

    // Calculate a point along the ray at parameter t
    // P(t) = origin + t * direction
    vec3 at(float t) const {
        return origin + direction * t;
    }

private:
    vec3 origin;       // Starting point of the ray
    vec3 direction;    // Direction vector (normalized)
};

#endif // RAY_H
