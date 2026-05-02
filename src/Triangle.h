#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "vec.h"
#include "Ray.h"
#include "HitRecord.h"
#include "AABB.h"
#include <array>
#include <optional>
#include <cmath>

class Triangle {
public:
    // Constructor with three vertices and material — face normal only (flat shading).
    Triangle(const vec3& v0, const vec3& v1, const vec3& v2, const Material& material_color)
        : v0(v0), v1(v1), v2(v2), material(material_color), has_vertex_normals(false) {
        edge1 = v1 - v0;
        edge2 = v2 - v0;
        face_normal = edge1.cross(edge2).normalized();
    }

    // Constructor with three vertices, three per-vertex normals, and material.
    // The hit record returns the barycentric-interpolated normal, giving smooth shading
    // across mesh facets. Vertex normals are renormalized after interpolation.
    Triangle(const vec3& v0, const vec3& v1, const vec3& v2,
             const vec3& n0, const vec3& n1, const vec3& n2,
             const Material& material_color)
        : v0(v0), v1(v1), v2(v2), material(material_color), has_vertex_normals(true) {
        edge1 = v1 - v0;
        edge2 = v2 - v0;
        face_normal = edge1.cross(edge2).normalized();
        vertex_normals[0] = n0;
        vertex_normals[1] = n1;
        vertex_normals[2] = n2;
    }

    // Möller–Trumbore ray-triangle intersection.
    std::optional<HitRecord> intersect(const Ray& ray, float t_min = 0.001f, float t_max = 1e6f) const {
        const float EPSILON = 1e-8f;

        vec3 h = ray.getDirection().cross(edge2);
        float a = edge1.dot(h);

        if (std::abs(a) < EPSILON) {
            return std::nullopt;
        }

        float f = 1.0f / a;
        vec3 s = ray.getOrigin() - v0;
        float u = f * s.dot(h);

        if (u < 0.0f || u > 1.0f) {
            return std::nullopt;
        }

        vec3 q = s.cross(edge1);
        float v = f * ray.getDirection().dot(q);

        if (v < 0.0f || u + v > 1.0f) {
            return std::nullopt;
        }

        float t = f * edge2.dot(q);

        if (t < t_min || t > t_max) {
            return std::nullopt;
        }

        vec3 point = ray.at(t);

        // Pick normal: barycentric-interpolated vertex normals (if provided), else face normal.
        vec3 hit_normal = face_normal;
        if (has_vertex_normals) {
            float w = 1.0f - u - v;
            hit_normal = (vertex_normals[0] * w + vertex_normals[1] * u + vertex_normals[2] * v).normalized();
        }

        // is_front_face from the *geometric* (face) normal so it correctly classifies entry/exit
        // for refraction even when interpolated normals would disagree near silhouette edges.
        bool is_front_face = ray.getDirection().dot(face_normal) < 0.0f;

        return HitRecord{t, point, hit_normal, material, is_front_face};
    }

    // Axis-aligned bounding box over the three vertices, padded by a tiny
    // epsilon so axis-aligned triangles (zero extent on one axis) still produce
    // a non-degenerate slab for BVH traversal.
    AABB bounds() const {
        AABB box;
        box.expand(v0);
        box.expand(v1);
        box.expand(v2);
        const float eps = 1e-4f;
        for (int i = 0; i < 3; ++i) {
            if (box.max[i] - box.min[i] < eps) {
                box.min[i] -= eps * 0.5f;
                box.max[i] += eps * 0.5f;
            }
        }
        return box;
    }

    vec3 getV0() const { return v0; }
    vec3 getV1() const { return v1; }
    vec3 getV2() const { return v2; }
    Material getMaterial() const { return material; }
    vec3 getNormal() const { return face_normal; }

private:
    vec3 v0, v1, v2;
    Material material;
    vec3 edge1, edge2;
    vec3 face_normal;
    bool has_vertex_normals;
    std::array<vec3, 3> vertex_normals;
};

#endif // TRIANGLE_H
