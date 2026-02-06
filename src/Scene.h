#ifndef SCENE_H
#define SCENE_H

#include "Sphere.h"
#include "Light.h"
#include <vector>
#include <optional>

class Scene {
public:
    // Add a sphere to the scene
    void addSphere(const Sphere& sphere) {
        spheres.push_back(sphere);
    }

    // Add a light to the scene
    void addLight(const Light& light) {
        lights.push_back(light);
    }

    // Get lights in the scene
    const std::vector<Light>& getLights() const {
        return lights;
    }

    // Trace a ray through the scene and return the color
    // If ray intersects an object, returns the material color of the closest object
    // If no intersection, returns background color
    vec3 traceRay(const Ray& ray, const vec3& background_color) const {
        float closest_t = 1e6f;
        std::optional<HitRecord> closest_hit;

        // Check intersection with all spheres
        for (const auto& sphere : spheres) {
            auto hit = sphere.intersect(ray, 0.001f, closest_t);
            if (hit.has_value()) {
                closest_t = hit->t;
                closest_hit = hit;
            }
        }

        // Return material color if we hit something, otherwise background
        if (closest_hit.has_value()) {
            return closest_hit->material;
        } else {
            return background_color;
        }
    }

    // Trace a ray through the scene and return the full hit record
    // Returns optional HitRecord if intersection occurs, nullopt otherwise
    std::optional<HitRecord> traceRayWithHitInfo(const Ray& ray) const {
        float closest_t = 1e6f;
        std::optional<HitRecord> closest_hit;

        // Check intersection with all spheres
        for (const auto& sphere : spheres) {
            auto hit = sphere.intersect(ray, 0.001f, closest_t);
            if (hit.has_value()) {
                closest_t = hit->t;
                closest_hit = hit;
            }
        }

        return closest_hit;
    }

    // Get number of objects in scene
    size_t getObjectCount() const {
        return spheres.size();
    }

private:
    std::vector<Sphere> spheres;
    std::vector<Light> lights;
};

#endif // SCENE_H
