#ifndef SCENEPRESETS_H
#define SCENEPRESETS_H

#include "Scene.h"
#include "Sphere.h"
#include "Light.h"
#include "vec.h"

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

        return scene;
    }
};

#endif // SCENEPRESETS_H
