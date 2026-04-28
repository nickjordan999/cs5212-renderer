#ifndef OPTICS_H
#define OPTICS_H

#include <cmath>
#include <optional>
#include "vec.h"
#include "Material.h"

// Snell's law refraction.
// v: incident direction (unit, pointing toward the surface).
// n: surface normal (unit, oriented against v — i.e. pointing back toward the side v came from).
// eta: ratio of indices of refraction, n_incident / n_transmitted.
// Returns nullopt on total internal reflection.
inline std::optional<vec3> refract(const vec3& v, const vec3& n, float eta) {
    float cos_i = std::min(1.0f, std::max(-1.0f, -v.dot(n)));
    float sin2_t = eta * eta * (1.0f - cos_i * cos_i);
    if (sin2_t > 1.0f) return std::nullopt;
    float cos_t = std::sqrt(1.0f - sin2_t);
    return (v * eta + n * (eta * cos_i - cos_t)).normalized();
}

// Schlick's Fresnel approximation. cos_theta is the cosine of the angle between
// the *incident* ray and the surface normal (oriented against the ray).
// Returns reflectance kr in [0, 1]; transmittance is (1 - kr).
// Handles total internal reflection by substituting cos_t when transmitting from
// dense to thin medium.
inline float fresnelSchlick(float cos_theta, float ior_from, float ior_to) {
    float r0 = (ior_from - ior_to) / (ior_from + ior_to);
    r0 = r0 * r0;
    float cos_use = cos_theta;
    if (ior_from > ior_to) {
        // Going from dense to thin: account for the larger transmitted angle.
        float eta = ior_from / ior_to;
        float sin2_t = eta * eta * (1.0f - cos_theta * cos_theta);
        if (sin2_t > 1.0f) return 1.0f;  // total internal reflection
        cos_use = std::sqrt(1.0f - sin2_t);
    }
    float one_minus = 1.0f - cos_use;
    return r0 + (1.0f - r0) * one_minus * one_minus * one_minus * one_minus * one_minus;
}

#endif // OPTICS_H
