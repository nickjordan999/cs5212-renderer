#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "vec.h"
#include "Ray.h"
#include "HitRecord.h"
#include <optional>
#include <cmath>

class Triangle {
public:
    // Constructor with three vertices and material color
    Triangle(const vec3& v0, const vec3& v1, const vec3& v2, const Material& material_color)
        : v0(v0), v1(v1), v2(v2), material(material_color) {
        // Pre-compute edges for intersection calculations
        edge1 = v1 - v0;
        edge2 = v2 - v0;
        // Pre-compute face normal
        face_normal = edge1.cross(edge2).normalized();
    }

    // Möller–Trumbore ray-triangle intersection algorithm
    // Returns HitRecord if intersection occurs within [t_min, t_max], nullopt otherwise
    std::optional<HitRecord> intersect(const Ray& ray, float t_min = 0.001f, float t_max = 1e6f) const {
        const float EPSILON = 1e-8f;
        
        // Compute ray-plane intersection using cross products
        vec3 h = ray.getDirection().cross(edge2);
        float a = edge1.dot(h);

        // If a is close to zero, ray is parallel to triangle
        if (std::abs(a) < EPSILON) {
            return std::nullopt;
        }

        float f = 1.0f / a;
        vec3 s = ray.getOrigin() - v0;
        float u = f * s.dot(h);

        // Check if intersection point is outside triangle bounds
        if (u < 0.0f || u > 1.0f) {
            return std::nullopt;
        }

        vec3 q = s.cross(edge1);
        float v = f * ray.getDirection().dot(q);

        // Check if intersection point is outside triangle bounds
        if (v < 0.0f || u + v > 1.0f) {
            return std::nullopt;
        }

        // Compute t to find intersection point along ray
        float t = f * edge2.dot(q);

        // Check if intersection is within valid range
        if (t < t_min || t > t_max) {
            return std::nullopt;
        }

        // Calculate intersection point
        vec3 point = ray.at(t);

        // Return hit record with face normal
        // For single-sided triangles, we use the face normal consistently
        return HitRecord{
            t,
            point,
            face_normal,
            material
        };
    }

    // Getters
    vec3 getV0() const { return v0; }
    vec3 getV1() const { return v1; }
    vec3 getV2() const { return v2; }
    Material getMaterial() const { return material; }
    vec3 getNormal() const { return face_normal; }

private:
    vec3 v0, v1, v2;        // Triangle vertices
    Material material;      // Material properties
    vec3 edge1, edge2;      // Pre-computed edges (v1-v0, v2-v0)
    vec3 face_normal;       // Pre-computed face normal
};

#endif // TRIANGLE_H
