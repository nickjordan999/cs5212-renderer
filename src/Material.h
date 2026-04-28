#ifndef MATERIAL_H
#define MATERIAL_H

#include "vec.h"
#include <algorithm>
#include <cmath>
#include <optional>

enum class ShaderType { SIMPLE, NORMAL, DIFFUSE, BLINN_PHONG, MIRROR, PATH_DIFFUSE, DIELECTRIC };

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
    vec3 color;
    std::optional<ShaderType> shaderType = std::nullopt;
    float ior = 1.0f;                                    // refractive index (1.0 = air, 1.33 = water, 1.5 = glass)
    vec3 transmission = vec3(1.0f, 1.0f, 1.0f);          // tint applied to refracted radiance
    bool is_caustic_receiver = false;                    // marks surfaces that should sample the caustics map

    // Default: diffuse material from color
    Material(const vec3& c) : color(c) {}

    // Material with explicit shader override
    Material(const vec3& c, ShaderType shader) : color(c), shaderType(shader) {}

    // Perfect mirror — reflects all light with no tint
    static Material Mirror() {
        return Material(vec3(1.0f, 1.0f, 1.0f), ShaderType::MIRROR);
    }

    // Water dielectric: ior=1.33, slight blue/cyan tint on transmitted light
    static Material Water() {
        Material m(vec3(0.85f, 0.95f, 1.0f), ShaderType::DIELECTRIC);
        m.ior = 1.33f;
        m.transmission = vec3(0.85f, 0.95f, 1.0f);
        return m;
    }

    // Generic glass dielectric: ior=1.5, no tint
    static Material Glass(float ior = 1.5f) {
        Material m(vec3(1.0f, 1.0f, 1.0f), ShaderType::DIELECTRIC);
        m.ior = ior;
        m.transmission = vec3(1.0f, 1.0f, 1.0f);
        return m;
    }
};

inline vec3 reflect(const vec3& v, const vec3& n) {
    return v - n * 2.0f * v.dot(n);
}

#endif // MATERIAL_H
