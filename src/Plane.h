#ifndef PLANE_H
#define PLANE_H

#include "vec.h"
#include "Ray.h"
#include "HitRecord.h"
#include <optional>
#include <cmath>

class Plane {
public:
    enum Pattern { SOLID, CHECKER, HEX };

    // Solid-color plane
    Plane(const vec3& point, const vec3& normal, const Material& material)
        : point(point), normal(normal.normalized()), mat1(material), mat2(material),
          pattern(SOLID), scale(1.0f) {}

    // Patterned plane — alternates mat1/mat2 based on pattern type
    // Default pattern is CHECKER for backward compatibility
    Plane(const vec3& point, const vec3& normal, const Material& mat1, const Material& mat2,
          float scale, Pattern pattern = CHECKER)
        : point(point), normal(normal.normalized()), mat1(mat1), mat2(mat2),
          pattern(pattern), scale(scale) {}

    // Ray-plane intersection
    // Plane equation: dot(P - point, normal) = 0
    // Substituting ray P(t) = O + t*D:
    //   dot(O + t*D - point, normal) = 0
    //   t = dot(point - O, normal) / dot(D, normal)
    std::optional<HitRecord> intersect(const Ray& ray, float t_min = 0.001f, float t_max = 1e6f) const {
        float denom = ray.getDirection().dot(normal);

        // Ray parallel to plane
        if (std::abs(denom) < 1e-8f) {
            return std::nullopt;
        }

        float t = (point - ray.getOrigin()).dot(normal) / denom;

        if (t < t_min || t > t_max) {
            return std::nullopt;
        }

        vec3 hit_point = ray.at(t);

        // Flip normal to face the ray (double-sided)
        vec3 outward_normal = (denom < 0.0f) ? normal : normal * -1.0f;

        // Pick material based on pattern
        Material hit_mat = mat1;
        if (pattern == CHECKER) {
            int ix = static_cast<int>(std::floor(hit_point[0] / scale));
            int iz = static_cast<int>(std::floor(hit_point[2] / scale));
            if ((ix + iz) % 2 != 0) {
                hit_mat = mat2;
            }
        } else if (pattern == HEX) {
            float x = hit_point[0];
            float z = hit_point[2];
            float sqrt3 = std::sqrt(3.0f);

            // Convert to fractional axial coordinates (flat-top hexes, size = scale)
            float fq = (2.0f / 3.0f * x) / scale;
            float fr = (-1.0f / 3.0f * x + sqrt3 / 3.0f * z) / scale;
            float fs = -fq - fr;

            // Cube-round to nearest hex cell
            int qi = static_cast<int>(std::round(fq));
            int ri = static_cast<int>(std::round(fr));
            int si = static_cast<int>(std::round(fs));

            float q_diff = std::abs(fq - qi);
            float r_diff = std::abs(fr - ri);
            float s_diff = std::abs(fs - si);

            if (q_diff > r_diff && q_diff > s_diff)
                qi = -ri - si;
            else if (r_diff > s_diff)
                ri = -qi - si;

            // Alternating columns for a checkerboard-like 2-coloring
            if (((qi % 2) + 2) % 2 != 0) {
                hit_mat = mat2;
            }
        }

        // Plane is double-sided and we already flipped to face the ray, so this is always
        // a "front face" hit by construction.
        return HitRecord{t, hit_point, outward_normal, hit_mat, true};
    }

private:
    vec3 point;       // A point on the plane
    vec3 normal;      // Plane normal (stored normalized)
    Material mat1;    // Primary material (or pattern color 1)
    Material mat2;    // Secondary material (pattern color 2)
    Pattern pattern;
    float scale;      // Tile size
};

#endif // PLANE_H
