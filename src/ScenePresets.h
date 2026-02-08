#ifndef SCENEPRESETS_H
#define SCENEPRESETS_H

#include "Scene.h"
#include "Camera.h"
#include "Sphere.h"
#include "Light.h"
#include "vec.h"
#include <memory>
#include <random>

// Utility class for creating pre-configured scenes
class ScenePresets {
public:
    // Create a simple test scene with colored spheres
    static Scene createTestScene() {
        Scene scene;

        // Large central sphere
        scene.addSphere(Sphere(vec3(0.0f, 0.0f, -5.0f), 1.0f, vec3(1.0f, 0.0f, 0.0f)));     // Red sphere

        // Medium spheres to the sides
        scene.addSphere(Sphere(vec3(-2.5f, 0.0f, -6.0f), 0.667f, vec3(0.0f, 1.0f, 0.0f)));    // Green sphere
        scene.addSphere(Sphere(vec3(2.5f, 0.0f, -6.0f), 0.667f, vec3(0.0f, 0.0f, 1.0f)));     // Blue sphere

        // Small spheres on the top and bottom
        scene.addSphere(Sphere(vec3(0.0f, -2.0f, -6.0f), 0.333f, vec3(1.0f, 1.0f, 1.0f)));    // White sphere
        scene.addSphere(Sphere(vec3(0.0f, 2.0f, -6.0f), 0.333f, vec3(1.0f, 1.0f, 0.0f)));     // Yellow sphere

        // Add default camera
        scene.setCamera(std::make_shared<PerspectiveBasicCamera>(
            vec3(0.0f, 0.0f, 3.0f), vec3(0.0f, 0.0f, -1.0f), 0.25f, 800.0f, 600.0f
        ));

        return scene;
    }

    // Create a test scene with lighting for diffuse rendering
    static Scene createLitTestScene() {
        Scene scene = createTestScene();

        // Add lights to the scene
        scene.addLight(Light(vec3(5.0f, 5.0f, 5.0f), vec3(1.0f, 1.0f, 1.0f)));      // Main light
        scene.addLight(Light(vec3(-5.0f, 3.0f, 4.0f), vec3(0.5f, 0.5f, 0.5f)));     // Fill light

        return scene;
    }

    // Create a simple scene with a single sphere
    static Scene createSingleSphereScene() {
        Scene scene;
        scene.addSphere(Sphere(vec3(0.0f, 0.0f, -5.0f), 1.5f, vec3(0.8f, 0.2f, 0.9f)));  // Purple sphere
        
        // Add default camera
        scene.setCamera(std::make_shared<PerspectiveBasicCamera>(
            vec3(0.0f, 0.0f, 3.0f), vec3(0.0f, 0.0f, -1.0f), 0.25f, 800.0f, 600.0f
        ));
        
        return scene;
    }

    // Create a scene with multiple spheres in a grid
    static Scene createGridScene() {
        Scene scene;

        // Create a 3x3 grid of spheres
        float spacing = 2.0f;
        float radius = 0.4f;
        float z = -8.0f;

        for (int i = -1; i <= 1; ++i) {
            for (int j = -1; j <= 1; ++j) {
                float x = i * spacing;
                float y = j * spacing;
                
                // Vary colors based on position
                float r = (i + 1.5f) / 3.0f;
                float g = (j + 1.5f) / 3.0f;
                float b = 0.5f + 0.5f * ((i + j + 2.0f) / 4.0f);
                
                scene.addSphere(Sphere(vec3(x, y, z), radius, vec3(r, g, b)));
            }
        }

        // Add default camera
        scene.setCamera(std::make_shared<PerspectiveBasicCamera>(
            vec3(0.0f, 0.0f, 3.0f), vec3(0.0f, 0.0f, -1.0f), 0.25f, 800.0f, 600.0f
        ));

        return scene;
    }

    // Create a scene with 50 spheres lined up on the z-axis with random colors
    static Scene createAlignedSpheresScene() {
        Scene scene;

        // Random number generation for colors
        std::mt19937 rng(42);  // Fixed seed for reproducibility
        std::uniform_real_distribution<float> colorDist(0.2f, 1.0f);  // Avoid too dark colors

        // Create 50 spheres of radius 1, kissing along the z-axis
        const int numSpheres = 50;
        const float radius = 1.0f;
        const float spacing = 2.0f * radius;  // Spheres kiss when spacing equals 2*radius

        for (int i = 0; i < numSpheres; ++i) {
            // Position along z-axis, centered around z = -50
            float z = -100.0f + i * spacing;

            // Generate random color
            float r = colorDist(rng);
            float g = colorDist(rng);
            float b = colorDist(rng);

            scene.addSphere(Sphere(vec3(0.0f, 0.0f, z), radius, vec3(r, g, b)));
        }

        // Add default camera positioned to view the line of spheres
        // Position camera further back to view all 50 spheres
        scene.setCamera(std::make_shared<PerspectiveBasicCamera>(
            vec3(5.0f, 5.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f), 1.0f, 400.0f, 400.0f
        ));

        scene.addLight(Light(vec3(5.0f, 5.0f, 5.0f), vec3(1.0f, 1.0f, 1.0f)));

        return scene;
    }

    // Create a scene with 50 spheres spiraling around the z-axis with random colors
    static Scene createSpiralSpheresScene() {
        Scene scene;

        // Random number generation for colors
        std::mt19937 rng(42);  // Fixed seed for reproducibility
        std::uniform_real_distribution<float> colorDist(0.2f, 1.0f);  // Avoid too dark colors

        // Create 50 spheres of radius 1, spiraling around the z-axis
        const int numSpheres = 500;
        const float radius = 1.0f;
        const float spacing = 2.0f * radius;  // Spheres kiss when spacing equals 2*radius
        const float spiralRadius = 5.0f;  // Radius of the spiral
        const float spiralTurns = 75.0f;  // Number of complete turns

        for (int i = 0; i < numSpheres; ++i) {
            // Position along z-axis
            float z = -i * spacing;

            // Calculate angle for spiral (increases with each sphere)
            float angle = (i / static_cast<float>(numSpheres)) * 2.0f * 3.14159265359f * spiralTurns;

            // Position on spiral
            float x = spiralRadius * std::cos(angle);
            float y = spiralRadius * std::sin(angle);

            // Generate random color
            float r = colorDist(rng);
            float g = colorDist(rng);
            float b = colorDist(rng);

            scene.addSphere(Sphere(vec3(x, y, z), radius, vec3(r, g, b)));
        }

        // Add default camera positioned to view the spiral
        scene.setCamera(std::make_shared<PerspectiveBasicCamera>(
            vec3(0.0f, 0.0f, -5.0f), vec3(0.0f, 0.0f, -1.0f), 5.0f, 400.0f, 400.0f
        ));

        scene.addLight(Light(vec3(10.0f, 10.0f, 10.0f), vec3(1.0f, 1.0f, 1.0f)));

        return scene;
    }
};

#endif // SCENEPRESETS_H
