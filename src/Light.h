#ifndef LIGHT_H
#define LIGHT_H

#include "vec.h"

class Light {
public:
    // Constructor for a point light
    // position: position of the light in world space
    // intensity: intensity/color of the light
    Light(const vec3& position, const vec3& intensity)
        : position(position), intensity(intensity) {}

    // Get light position
    const vec3& getPosition() const { return position; }

    // Get light intensity
    const vec3& getIntensity() const { return intensity; }

private:
    vec3 position;
    vec3 intensity;
};

#endif // LIGHT_H
