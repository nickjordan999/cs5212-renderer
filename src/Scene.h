#ifndef SCENE_H
#define SCENE_H

#include "Sphere.h"
#include "Triangle.h"
#include "Plane.h"
#include "Light.h"
#include "Camera.h"
#include "Solid.h"
#include "BVH.h"
#include <vector>
#include <optional>
#include <memory>

class Scene {
public:
    enum class BackgroundMode { None, Solid, Gradient };

    Scene() : camera(nullptr), bgMode(BackgroundMode::None),
              bgTop(0.0f, 0.0f, 0.0f), bgHorizon(0.0f, 0.0f, 0.0f), bgBottom(0.0f, 0.0f, 0.0f),
              bgUp(0.0f, 1.0f, 0.0f) {}

    // Set a flat background color for this scene
    void setBackgroundColor(const vec3& color) {
        bgMode = BackgroundMode::Solid;
        bgTop = bgHorizon = bgBottom = color;
    }

    // Three-color vertical gradient. The visible color is chosen by projecting the ray
    // direction onto up_axis: t = dir · up ∈ [-1, 1]. t > 0 mixes horizon→top; t < 0
    // mixes horizon→bottom. up_axis is normalized internally.
    void setBackgroundGradient(const vec3& top, const vec3& horizon, const vec3& bottom,
                               const vec3& up_axis) {
        bgMode = BackgroundMode::Gradient;
        bgTop = top; bgHorizon = horizon; bgBottom = bottom;
        bgUp = up_axis.normalized();
    }

    // Sample background for a ray. Returns fallback if scene has no background set.
    vec3 backgroundFor(const Ray& ray, const vec3& fallback) const {
        if (bgMode == BackgroundMode::None) return fallback;
        if (bgMode == BackgroundMode::Solid) return bgTop;
        float t = ray.getDirection().dot(bgUp);
        if (t >= 0.0f) return bgHorizon * (1.0f - t) + bgTop * t;
        return bgHorizon * (1.0f + t) + bgBottom * (-t);
    }

    // Solid-color accessor retained for callers that only need a flat color.
    std::optional<vec3> getBackgroundColor() const {
        if (bgMode == BackgroundMode::Solid) return bgTop;
        return std::nullopt;
    }

    // Set the camera for this scene
    void setCamera(std::shared_ptr<Camera> cam) {
        camera = cam;
    }

    // Get the camera for this scene
    const Camera& getCamera() const {
        if (!camera) {
            throw std::runtime_error("Scene has no camera set");
        }
        return *camera;
    }

    // Check if a camera is set
    bool hasCamera() const {
        return camera != nullptr;
    }
    // Add a sphere to the scene
    void addSphere(const Sphere& sphere) {
        spheres.push_back(sphere);
    }

    // Add a triangle to the scene
    void addTriangle(const Triangle& triangle) {
        triangles.push_back(triangle);
    }

    // Add a plane to the scene
    void addPlane(const Plane& plane) {
        planes.push_back(plane);
    }

    // Add a solid (platonic solid) to the scene as triangles
    void addSolid(const Solid& solid, const vec3& center, float scale, const Material& mat) {
        const auto& verts = solid.getVertices();
        const auto& faces = solid.getFaces();
        for (int i = 0; i < solid.faceCount(); ++i) {
            vec3 v0 = verts[faces[i*3+0]] * scale + center;
            vec3 v1 = verts[faces[i*3+1]] * scale + center;
            vec3 v2 = verts[faces[i*3+2]] * scale + center;
            // Ensure normal points outward (away from solid center)
            vec3 fc = (v0 + v1 + v2) / 3.0f;
            vec3 n = (v1 - v0).cross(v2 - v0);
            if (n.dot(fc - center) < 0)
                addTriangle(Triangle(v0, v2, v1, mat));
            else
                addTriangle(Triangle(v0, v1, v2, mat));
        }
    }

    // Add a light to the scene
    void addLight(const Light& light) {
        lights.push_back(light);
    }

    // Get lights in the scene
    const std::vector<Light>& getLights() const {
        return lights;
    }

    // Build the triangle BVH. Call once after all primitives have been added
    // (e.g. after a preset finishes populating the scene). Spheres and planes
    // are still scanned linearly — usually a small handful per scene, so the
    // BVH overhead isn't worth it for them.
    void buildAccelerator() {
        triangle_bvh.build(triangles);
    }

    // Test whether any object occludes the ray within [0.001, maxDist).
    // Used for shadow rays: origin is a surface point, direction points at a light,
    // and maxDist is the distance to that light.
    bool isOccluded(const Ray& ray, float maxDist) const {
        constexpr float t_min = 0.001f;
        for (const auto& sphere : spheres) {
            if (sphere.intersect(ray, t_min, maxDist).has_value()) return true;
        }
        if (triangle_bvh.isOccluded(ray, t_min, maxDist, triangles)) return true;
        for (const auto& plane : planes) {
            if (plane.intersect(ray, t_min, maxDist).has_value()) return true;
        }
        return false;
    }

    // Trace a ray through the scene and return the color
    // If ray intersects an object, returns the material color of the closest object
    // If no intersection, returns background color
    vec3 traceRay(const Ray& ray, const vec3& background_color) const {
        float closest_t = 1e6f;
        std::optional<HitRecord> closest_hit;

        for (const auto& sphere : spheres) {
            auto hit = sphere.intersect(ray, 0.001f, closest_t);
            if (hit.has_value()) {
                closest_t = hit->t;
                closest_hit = hit;
            }
        }

        // BVH narrows closest_t in place as it finds triangle hits.
        auto tri_hit = triangle_bvh.closestHit(ray, 0.001f, closest_t, triangles);
        if (tri_hit.has_value()) closest_hit = tri_hit;

        for (const auto& plane : planes) {
            auto hit = plane.intersect(ray, 0.001f, closest_t);
            if (hit.has_value()) {
                closest_t = hit->t;
                closest_hit = hit;
            }
        }

        if (closest_hit.has_value()) {
            return closest_hit->material.color;
        } else {
            return background_color;
        }
    }

    // Trace a ray through the scene and return the full hit record
    // Returns optional HitRecord if intersection occurs, nullopt otherwise
    std::optional<HitRecord> traceRayWithHitInfo(const Ray& ray) const {
        float closest_t = 1e6f;
        std::optional<HitRecord> closest_hit;

        for (const auto& sphere : spheres) {
            auto hit = sphere.intersect(ray, 0.001f, closest_t);
            if (hit.has_value()) {
                closest_t = hit->t;
                closest_hit = hit;
            }
        }

        auto tri_hit = triangle_bvh.closestHit(ray, 0.001f, closest_t, triangles);
        if (tri_hit.has_value()) closest_hit = tri_hit;

        for (const auto& plane : planes) {
            auto hit = plane.intersect(ray, 0.001f, closest_t);
            if (hit.has_value()) {
                closest_t = hit->t;
                closest_hit = hit;
            }
        }

        return closest_hit;
    }

    // Get number of objects in scene
    size_t getObjectCount() const {
        return spheres.size() + triangles.size() + planes.size();
    }

    const std::vector<Sphere>& getSpheres() const { return spheres; }
    const std::vector<Triangle>& getTriangles() const { return triangles; }
    const std::vector<Plane>& getPlanes() const { return planes; }

private:
    std::vector<Sphere> spheres;
    std::vector<Triangle> triangles;
    std::vector<Plane> planes;
    std::vector<Light> lights;
    std::shared_ptr<Camera> camera;
    BackgroundMode bgMode;
    vec3 bgTop, bgHorizon, bgBottom;
    vec3 bgUp;
    BVH triangle_bvh;  // Built by buildAccelerator() after triangles populated.
};

#endif // SCENE_H
