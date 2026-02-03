#ifndef SCENE_H
#define SCENE_H

#include "Sphere.h"
#include <vector>
#include <optional>

class Scene {
public:
    // Add a sphere to the scene
    void addSphere(const Sphere& sphere) {
        spheres.push_back(sphere);
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

    // Get number of objects in scene
    size_t getObjectCount() const {
        return spheres.size();
    }

private:
    std::vector<Sphere> spheres;
};

#endif // SCENE_H
