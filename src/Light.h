#ifndef LIGHT_H
#define LIGHT_H

#include "vec.h"

class Light {
public:
    // Constructor for a point light
    // position: position of the light in world space
    // intensity: intensity/color of the light used for direct shading
    // photon_scale: multiplier applied to intensity for photon emission only
    //   (lets a scene boost caustic flux without washing out direct shading)
    Light(const vec3& position, const vec3& intensity, float photon_scale = 1.0f)
        : position(position), intensity(intensity), photon_scale(photon_scale) {}

    // Get light position
    const vec3& getPosition() const { return position; }

    // Get light intensity used for direct shading
    const vec3& getIntensity() const { return intensity; }

    // Get effective intensity for photon emission (caustics pre-pass)
    vec3 getPhotonIntensity() const { return intensity * photon_scale; }

private:
    vec3 position;
    vec3 intensity;
    float photon_scale;
};

#endif // LIGHT_H
