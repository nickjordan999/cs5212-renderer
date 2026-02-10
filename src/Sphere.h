#ifndef SPHERE_H
#define SPHERE_H

#include "vec.h"
#include "Ray.h"
#include "HitRecord.h"
#include <optional>
#include <cmath>

class Sphere {
public:
    // Constructor
    // center: center of sphere in world space
    // radius: radius of sphere
    // material: material color of sphere
    Sphere(const vec3& center, float radius, const Material& material)
        : center(center), radius(radius), material(material) {}

    // Get sphere center
    const vec3& getCenter() const { return center; }

    // Get sphere radius
    float getRadius() const { return radius; }

    // Get material
    const Material& getMaterial() const { return material; }

    // Check if ray intersects sphere and return hit information
    // Returns optional HitRecord if intersection occurs, nullopt otherwise
    std::optional<HitRecord> intersect(const Ray& ray, float t_min = 0.001f, float t_max = 1e6f) const {
        // Ray: P(t) = O + t*D where O is origin, D is direction (normalized)
        // Sphere: |P - C|^2 = r^2 where C is center, r is radius
        // 
        // Substituting ray into sphere equation:
        // |O + t*D - C|^2 = r^2
        // Let V = O - C:
        // |V + t*D|^2 = r^2
        // (V + t*D) · (V + t*D) = r^2
        // V·V + 2t(V·D) + t^2(D·D) = r^2
        //
        // Rearranging as quadratic in t:
        // t^2(D·D) + 2t(V·D) + (V·V - r^2) = 0
        // Since D is normalized, D·D = 1, so:
        // t^2 + 2t(V·D) + (V·V - r^2) = 0
        // 
        // Using quadratic formula: t = (-b ± sqrt(b^2 - 4ac)) / 2a
        // where a=1, b=2(V·D), c=(V·V - r^2)

        vec3 oc = ray.getOrigin() - center;
        vec3 d = ray.getDirection();

        float a = d.dot(d);  // Should be 1 since direction is normalized
        float b = 2.0f * oc.dot(d);
        float c = oc.dot(oc) - radius * radius;

        float discriminant = b * b - 4.0f * a * c;

        // No intersection if discriminant is negative
        if (discriminant < 0.0f) {
            return std::nullopt;
        }

        float sqrt_discriminant = std::sqrt(discriminant);
        float t1 = (-b - sqrt_discriminant) / (2.0f * a);
        float t2 = (-b + sqrt_discriminant) / (2.0f * a);

        // Choose the closest intersection point within valid range
        float t = -1.0f;
        if (t1 >= t_min && t1 <= t_max) {
            t = t1;
        } else if (t2 >= t_min && t2 <= t_max) {
            t = t2;
        } else {
            return std::nullopt;
        }

        // Calculate hit information
        vec3 point = ray.at(t);
        vec3 normal = (point - center).normalized();

        return HitRecord{t, point, normal, material};
    }

private:
    vec3 center;
    float radius;
    Material material;  // Material properties
};

#endif // SPHERE_H
