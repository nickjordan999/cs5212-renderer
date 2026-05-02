#ifndef AABB_H
#define AABB_H

#include "vec.h"
#include "Ray.h"
#include <algorithm>
#include <limits>

// Axis-aligned bounding box. Used by the BVH to cull whole subtrees of
// triangles when a ray misses their enclosing box.
struct AABB {
    vec3 min;
    vec3 max;

    AABB() {
        const float inf = std::numeric_limits<float>::infinity();
        min = vec3(inf, inf, inf);
        max = vec3(-inf, -inf, -inf);
    }

    AABB(const vec3& a, const vec3& b) : min(a), max(b) {}

    void expand(const vec3& p) {
        for (int i = 0; i < 3; ++i) {
            if (p[i] < min[i]) min[i] = p[i];
            if (p[i] > max[i]) max[i] = p[i];
        }
    }

    void expand(const AABB& other) {
        expand(other.min);
        expand(other.max);
    }

    vec3 centroid() const {
        return (min + max) * 0.5f;
    }

    int longest_axis() const {
        vec3 d = max - min;
        if (d[0] >= d[1] && d[0] >= d[2]) return 0;
        if (d[1] >= d[2]) return 1;
        return 2;
    }

    // Slab test. Returns true if the ray enters the box within [t_min, t_max].
    // Branchless on the per-axis swap; division-by-zero is well-defined for IEEE
    // floats (yields ±inf), and the subsequent compares correctly classify rays
    // parallel to a slab as either always-inside or always-outside that slab.
    bool intersect(const Ray& ray, float t_min, float t_max) const {
        const vec3& o = ray.getOrigin();
        const vec3& d = ray.getDirection();
        for (int i = 0; i < 3; ++i) {
            float invD = 1.0f / d[i];
            float t0 = (min[i] - o[i]) * invD;
            float t1 = (max[i] - o[i]) * invD;
            if (invD < 0.0f) std::swap(t0, t1);
            if (t0 > t_min) t_min = t0;
            if (t1 < t_max) t_max = t1;
            if (t_max <= t_min) return false;
        }
        return true;
    }
};

#endif // AABB_H
