#ifndef BVH_H
#define BVH_H

#include "AABB.h"
#include "Triangle.h"
#include "HitRecord.h"
#include "Ray.h"
#include <vector>
#include <optional>
#include <algorithm>

// Triangle-only bounding volume hierarchy. Built once after the scene's
// triangle list is finalized, then queried by Scene::traceRay* / isOccluded.
//
// Layout: a flat std::vector<BVHNode>. Internal nodes have prim_count == 0
// and reference children by index; leaves have prim_count > 0 and reference
// a contiguous range of prim_indices (a permutation of [0..N)).
//
// Build strategy: median split on the longest axis of the centroid bounds
// using std::nth_element (O(N) per partition, O(N log N) overall). Cheap to
// build, ~80% of the runtime win of SAH for far less code.
class BVH {
public:
    void build(const std::vector<Triangle>& tris) {
        nodes.clear();
        prim_indices.clear();
        if (tris.empty()) return;

        prim_indices.resize(tris.size());
        std::vector<AABB> tri_bounds(tris.size());
        std::vector<vec3> tri_centroids(tris.size());
        for (size_t i = 0; i < tris.size(); ++i) {
            prim_indices[i] = static_cast<int>(i);
            tri_bounds[i] = tris[i].bounds();
            tri_centroids[i] = tri_bounds[i].centroid();
        }

        // With LEAF_THRESHOLD >= 2, total nodes < 2N. Reserve so node
        // pointers/indices stay valid across recursive pushes.
        nodes.reserve(2 * tris.size() + 1);
        buildRecursive(0, static_cast<int>(tris.size()), tri_bounds, tri_centroids, 0);
    }

    bool empty() const { return nodes.empty(); }

    // Closest-hit traversal: walks the tree, prunes by AABB, and shrinks the
    // working t_max as primitive hits are found. Updates `closest_t` in place
    // so callers can keep narrowing across other primitive lists (spheres,
    // planes) afterward.
    std::optional<HitRecord> closestHit(const Ray& ray, float t_min, float& closest_t,
                                        const std::vector<Triangle>& tris) const {
        std::optional<HitRecord> best;
        if (nodes.empty()) return best;

        int stack[MAX_STACK];
        int sp = 0;
        stack[sp++] = 0;

        while (sp > 0) {
            const BVHNode& node = nodes[stack[--sp]];
            if (!node.bounds.intersect(ray, t_min, closest_t)) continue;

            if (node.prim_count > 0) {
                for (int i = 0; i < node.prim_count; ++i) {
                    int pi = prim_indices[node.prim_start + i];
                    auto hit = tris[pi].intersect(ray, t_min, closest_t);
                    if (hit.has_value()) {
                        closest_t = hit->t;
                        best = hit;
                    }
                }
            } else {
                stack[sp++] = node.left;
                stack[sp++] = node.right;
            }
        }
        return best;
    }

    // Any-hit traversal for shadow rays: returns true on the first primitive
    // hit found within [t_min, t_max).
    bool isOccluded(const Ray& ray, float t_min, float t_max,
                    const std::vector<Triangle>& tris) const {
        if (nodes.empty()) return false;

        int stack[MAX_STACK];
        int sp = 0;
        stack[sp++] = 0;

        while (sp > 0) {
            const BVHNode& node = nodes[stack[--sp]];
            if (!node.bounds.intersect(ray, t_min, t_max)) continue;

            if (node.prim_count > 0) {
                for (int i = 0; i < node.prim_count; ++i) {
                    int pi = prim_indices[node.prim_start + i];
                    if (tris[pi].intersect(ray, t_min, t_max).has_value()) return true;
                }
            } else {
                stack[sp++] = node.left;
                stack[sp++] = node.right;
            }
        }
        return false;
    }

private:
    struct BVHNode {
        AABB bounds;
        int left = -1;
        int right = -1;
        int prim_start = 0;   // valid when prim_count > 0
        int prim_count = 0;   // 0 -> internal node, >0 -> leaf
    };

    static constexpr int LEAF_THRESHOLD = 4;
    // MAX_DEPTH bounds the traversal stack at MAX_STACK. The recursion depth
    // is also capped to MAX_DEPTH at build time so well-balanced trees never
    // exceed it; pathological inputs (collinear centroids, etc.) fall back to
    // a leaf when centroid-bounds extent collapses.
    static constexpr int MAX_DEPTH = 64;
    static constexpr int MAX_STACK = MAX_DEPTH * 2;

    std::vector<BVHNode> nodes;
    std::vector<int> prim_indices;

    int buildRecursive(int begin, int end,
                       const std::vector<AABB>& tri_bounds,
                       const std::vector<vec3>& tri_centroids,
                       int depth) {
        int node_idx = static_cast<int>(nodes.size());
        nodes.emplace_back();

        AABB node_bounds;
        AABB centroid_bounds;
        for (int i = begin; i < end; ++i) {
            int pi = prim_indices[i];
            node_bounds.expand(tri_bounds[pi]);
            centroid_bounds.expand(tri_centroids[pi]);
        }
        nodes[node_idx].bounds = node_bounds;

        int count = end - begin;
        int axis = centroid_bounds.longest_axis();
        bool centroids_collapsed =
            (centroid_bounds.max[axis] - centroid_bounds.min[axis]) < 1e-6f;

        if (count <= LEAF_THRESHOLD || depth >= MAX_DEPTH || centroids_collapsed) {
            nodes[node_idx].prim_start = begin;
            nodes[node_idx].prim_count = count;
            return node_idx;
        }

        int mid = begin + count / 2;
        std::nth_element(prim_indices.begin() + begin,
                         prim_indices.begin() + mid,
                         prim_indices.begin() + end,
                         [&](int a, int b) {
                             return tri_centroids[a][axis] < tri_centroids[b][axis];
                         });

        int left = buildRecursive(begin, mid, tri_bounds, tri_centroids, depth + 1);
        int right = buildRecursive(mid, end, tri_bounds, tri_centroids, depth + 1);
        nodes[node_idx].left = left;
        nodes[node_idx].right = right;
        return node_idx;
    }
};

#endif // BVH_H
