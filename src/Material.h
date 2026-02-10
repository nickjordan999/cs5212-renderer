#ifndef MATERIAL_H
#define MATERIAL_H

#include "vec.h"
#include <algorithm>
#include <cmath>
#include <random>

struct HSLColor {
    float h; // hue [0, 360)
    float s; // saturation [0, 1]
    float l; // lightness [0, 1]

    HSLColor(float h, float s, float l) : h(h), s(s), l(l) {}

    vec3 toRgb() const {
        float c = (1.0f - std::fabs(2.0f * l - 1.0f)) * s;
        float hp = h / 60.0f;
        float x = c * (1.0f - std::fabs(std::fmod(hp, 2.0f) - 1.0f));
        float m = l - c / 2.0f;

        float r, g, b;
        if (hp < 1.0f)      { r = c; g = x; b = 0; }
        else if (hp < 2.0f) { r = x; g = c; b = 0; }
        else if (hp < 3.0f) { r = 0; g = c; b = x; }
        else if (hp < 4.0f) { r = 0; g = x; b = c; }
        else if (hp < 5.0f) { r = x; g = 0; b = c; }
        else                 { r = c; g = 0; b = x; }

        return vec3(r + m, g + m, b + m);
    }
};

struct Material {
    enum Type { DIFFUSE, METALLIC };
    Type type;
    vec3 color;
    float fuzziness; // 0.0 = perfect mirror, 1.0 = very rough (only used for METALLIC)

    // Default: diffuse material from color (backwards compatible)
    Material(const vec3& c) : type(DIFFUSE), color(c), fuzziness(0.0f) {}

    // Metallic constructor
    Material(const vec3& c, float fuzz)
        : type(METALLIC), color(c), fuzziness(std::min(fuzz, 1.0f)) {}

    // Perfect mirror — reflects all light with no tint or fuzziness
    static Material Mirror() {
        return Material(vec3(1.0f, 1.0f, 1.0f), 0.0f);
    }
};

inline vec3 reflect(const vec3& v, const vec3& n) {
    return v - n * 2.0f * v.dot(n);
}

inline vec3 random_in_unit_sphere() {
    static thread_local std::mt19937 gen(std::random_device{}());
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
    while (true) {
        vec3 p(dist(gen), dist(gen), dist(gen));
        if (p.length_squared() < 1.0f) return p;
    }
}

#endif // MATERIAL_H
