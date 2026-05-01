#ifndef SCENEPRESETS_H
#define SCENEPRESETS_H

#include "Scene.h"
#include "Solid.h"
#include "Camera.h"
#include "Sphere.h"
#include "Triangle.h"
#include "Plane.h"
#include "Light.h"
#include "Material.h"
#include "PerlinNoise.h"
#include "vec.h"
#include <memory>
#include <random>
#include <cmath>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <unordered_map>

using SceneParams = std::unordered_map<std::string, float>;

// Utility class for creating pre-configured scenes
class ScenePresets
{
public:
  // Create a simple test scene with colored spheres
  static Scene createTestScene()
  {
    Scene scene;

    // Large central sphere
    scene.addSphere(Sphere(vec3(0.0f, 0.0f, -5.0f), 1.0f, vec3(1.0f, 0.0f, 0.0f)));// Red sphere

    // Medium spheres to the sides
    scene.addSphere(Sphere(vec3(-2.5f, 0.0f, -6.0f), 0.667f, vec3(0.0f, 1.0f, 0.0f)));// Green sphere
    scene.addSphere(Sphere(vec3(2.5f, 0.0f, -6.0f), 0.667f, vec3(0.0f, 0.0f, 1.0f)));// Blue sphere

    // Small spheres on the top and bottom
    scene.addSphere(Sphere(vec3(0.0f, -2.0f, -6.0f), 0.333f, vec3(1.0f, 1.0f, 1.0f)));// White sphere
    scene.addSphere(Sphere(vec3(0.0f, 2.0f, -6.0f), 0.333f, vec3(1.0f, 1.0f, 0.0f)));// Yellow sphere

    // Add default camera
    scene.setCamera(std::make_shared<PerspectiveBasicCamera>(
      vec3(0.0f, 0.0f, 3.0f), vec3(0.0f, 0.0f, -1.0f), 0.25f));

    return scene;
  }

  // Create a test scene with lighting for diffuse rendering
  static Scene createLitTestScene()
  {
    Scene scene = createTestScene();

    // Add lights to the scene
    scene.addLight(Light(vec3(5.0f, 5.0f, 5.0f), vec3(1.0f, 1.0f, 1.0f)));// Main light
    scene.addLight(Light(vec3(-5.0f, 3.0f, 4.0f), vec3(0.5f, 0.5f, 0.5f)));// Fill light

    return scene;
  }

  // Create a simple scene with a single sphere
  static Scene createSingleSphereScene()
  {
    Scene scene;
    scene.addSphere(Sphere(vec3(0.0f, 0.0f, -5.0f), 1.5f, vec3(0.8f, 0.2f, 0.9f)));// Purple sphere

    // Add default camera
    scene.setCamera(std::make_shared<PerspectiveBasicCamera>(
      vec3(0.0f, 0.0f, 3.0f), vec3(0.0f, 0.0f, -1.0f), 0.25f));

    return scene;
  }

  // Create a scene with multiple spheres in a grid
  static Scene createGridScene()
  {
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
      vec3(0.0f, 0.0f, 3.0f), vec3(0.0f, 0.0f, -1.0f), 0.25f));

    return scene;
  }

  // Create a scene with 50 spheres lined up on the z-axis with random colors
  static Scene createAlignedSpheresScene()
  {
    Scene scene;

    // Random number generation for colors
    std::mt19937 rng(42);// Fixed seed for reproducibility
    std::uniform_real_distribution<float> colorDist(0.2f, 1.0f);// Avoid too dark colors

    // Create 50 spheres of radius 1, kissing along the z-axis
    const int numSpheres = 50;
    const float radius = 1.0f;
    const float spacing = 2.0f * radius;// Spheres kiss when spacing equals 2*radius

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
      vec3(5.0f, 5.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f), 1.0f));

    scene.addLight(Light(vec3(5.0f, 5.0f, 5.0f), vec3(1.0f, 1.0f, 1.0f)));

    return scene;
  }

  // Create a scene with spheres spiraling around the z-axis with random colors
  // Params:
  //   t            — fraction [0,1] positioning the camera along the spiral's z-extent
  //   num_spheres  — number of spheres (default 500)
  //   spiral_turns — number of complete turns (default 75)
  static Scene createSpiralSpheresScene(const SceneParams &params = {})
  {
    Scene scene;

    // Create spheres spiraling around the z-axis with HSL hue cycling
    const int numSpheres = params.count("num_spheres") ? static_cast<int>(params.at("num_spheres")) : 100;
    const float radius = 1.0f;
    const float spacing = 2.0f * radius;
    const float spiralRadius = 5.0f;
    const float spiralTurns = params.count("spiral_turns") ? params.at("spiral_turns") : 75.0f;

    scene.setBackgroundColor(vec3(0.0f, 0.0f, 0.0f));

    for (int i = 0; i < numSpheres; ++i) {
      // Position along z-axis
      float z = -i * spacing;

      // Calculate angle for spiral (increases with each sphere)
      float angle = (i / static_cast<float>(numSpheres)) * 2.0f * 3.14159265359f * spiralTurns;

      // Position on spiral
      float x = spiralRadius * std::cos(angle);
      float y = spiralRadius * std::sin(angle);

      float hue_shift = params.count("hue_shift") ? params.at("hue_shift") : 270.0f;// Cycle hue across all spheres, max saturation and lightness
      float hue = std::fmod((i / static_cast<float>(numSpheres)) * 360.0f + hue_shift, 360.f);
      vec3 color = HSLColor(hue, 1.0f, 0.5f).toRgb();

      scene.addSphere(Sphere(vec3(x, y, z), radius, Material(color)));
    }

    // Position camera along the spiral's z-extent based on parameter t
    float t = params.count("t") ? params.at("t") : 0.15f;
    float totalZ = (numSpheres - 1) * spacing;
    scene.setCamera(std::make_shared<PerspectiveBasicCamera>(
      vec3(0.0f, 0.0f, -t * totalZ), vec3(0.0f, 0.0f, -1.0f), 2.0f));

    scene.addLight(Light(vec3(0.0f, 0.0f, 0.0f), vec3(1.0f, 1.0f, 1.0f)));
    scene.addLight(Light(vec3(0.0f, 0.0f, -totalZ), vec3(1.0f, 1.0f, 1.0f)));

    return scene;
  }

  // Create an octahedron made of 8 triangles
  static Scene createOctahedronScene()
  {
    Scene scene;

    // An octahedron has 6 vertices: ±1 along each axis
    vec3 top(0.0f, 1.5f, -8.0f);// +Y
    vec3 bottom(0.0f, -1.5f, -8.0f);// -Y
    vec3 front(0.0f, 0.0f, -6.5f);// +Z (closer)
    vec3 back(0.0f, 0.0f, -9.5f);// -Z (farther)
    vec3 right(1.5f, 0.0f, -8.0f);// +X
    vec3 left(-1.5f, 0.0f, -8.0f);// -X

    // Materials for each face — mix of diffuse and mirror
    Material red(vec3(1.0f, 0.2f, 0.2f));
    Material green(vec3(0.2f, 1.0f, 0.2f));
    Material metalSilver(vec3(0.8f, 0.8f, 0.8f), ShaderType::MIRROR);
    Material metalGold(vec3(1.0f, 0.84f, 0.0f), ShaderType::MIRROR);
    Material cyan(vec3(0.2f, 1.0f, 1.0f));
    Material metalCopper(vec3(0.72f, 0.45f, 0.2f), ShaderType::MIRROR);
    Material orange(vec3(1.0f, 0.6f, 0.2f));
    Material metalBlue(vec3(0.4f, 0.4f, 0.9f), ShaderType::MIRROR);

    // Upper 4 faces (connecting top vertex to equatorial edges)
    scene.addTriangle(Triangle(top, front, right, red));
    scene.addTriangle(Triangle(top, right, back, metalSilver));
    scene.addTriangle(Triangle(top, back, left, metalGold));
    scene.addTriangle(Triangle(top, left, front, green));

    // Lower 4 faces (connecting bottom vertex to equatorial edges)
    scene.addTriangle(Triangle(bottom, right, front, cyan));
    scene.addTriangle(Triangle(bottom, back, right, metalCopper));
    scene.addTriangle(Triangle(bottom, left, back, orange));
    scene.addTriangle(Triangle(bottom, front, left, metalBlue));

    // Camera positioned to view the octahedron at an angle
    scene.setCamera(std::make_shared<PerspectiveBasicCamera>(
      vec3(4.0f, 3.0f, -2.0f), vec3(0.0f, 0.0f, -1.0f), 1.0f));

    // Lights
    scene.addLight(Light(vec3(5.0f, 5.0f, 5.0f), vec3(1.0f, 1.0f, 1.0f)));
    scene.addLight(Light(vec3(-5.0f, 3.0f, 4.0f), vec3(0.5f, 0.5f, 0.5f)));

    return scene;
  }

  // Create a scene with triangles - a simple pyramid
  static Scene createTrianglePyramidScene()
  {
    Scene scene;

    // Create a pyramid with a square base and apex
    // Base: z = -8, corners at (±2, ±2)
    // Apex: (0, 3, -8)

    // Define vertices
    vec3 apex(0.0f, 3.0f, -8.0f);
    vec3 baseV0(-2.0f, 0.0f, -8.0f);// Front-left
    vec3 baseV1(2.0f, 0.0f, -8.0f);// Front-right
    vec3 baseV2(2.0f, 0.0f, -12.0f);// Back-right
    vec3 baseV3(-2.0f, 0.0f, -12.0f);// Back-left

    // Define colors for each face
    vec3 colorRed(1.0f, 0.0f, 0.0f);
    vec3 colorGreen(0.0f, 1.0f, 0.0f);
    vec3 colorBlue(0.0f, 0.0f, 1.0f);
    vec3 colorYellow(1.0f, 1.0f, 0.0f);
    vec3 colorCyan(0.0f, 1.0f, 1.0f);

    // Front face: triangle (apex, baseV0, baseV1)
    scene.addTriangle(Triangle(apex, baseV0, baseV1, colorRed));

    // Right face: triangle (apex, baseV1, baseV2)
    scene.addTriangle(Triangle(apex, baseV1, baseV2, colorGreen));

    // Back face: triangle (apex, baseV2, baseV3)
    scene.addTriangle(Triangle(apex, baseV2, baseV3, colorBlue));

    // Left face: triangle (apex, baseV3, baseV0)
    scene.addTriangle(Triangle(apex, baseV3, baseV0, colorYellow));

    // Bottom base: two triangles (baseV0, baseV1, baseV2) and (baseV0, baseV2, baseV3)
    scene.addTriangle(Triangle(baseV0, baseV1, baseV2, colorCyan));
    scene.addTriangle(Triangle(baseV0, baseV2, baseV3, colorCyan));

    // Add default camera
    scene.setCamera(std::make_shared<PerspectiveBasicCamera>(
      vec3(5.0f, 5.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f), 1.0f));

    // Add lights for better visualization
    scene.addLight(Light(vec3(5.0f, 5.0f, 5.0f), vec3(1.0f, 1.0f, 1.0f)));
    scene.addLight(Light(vec3(-5.0f, 3.0f, 4.0f), vec3(0.5f, 0.5f, 0.5f)));

    return scene;
  }
  // Create a checkerboard floor with a mirror sphere above it
  static Scene createCheckerboardScene()
  {
    Scene scene;

    // Checkerboard floor at y=0, normal pointing up
    Material black(vec3(0.05f, 0.05f, 0.05f));
    Material white(vec3(0.95f, 0.95f, 0.95f));
    scene.addPlane(Plane(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), black, white, 2.0f));

    // Large mirror sphere floating above the plane
    scene.addSphere(Sphere(vec3(0.0f, 1.5f, -6.0f), 1.5f, Material::Mirror()));

    // Small colored diffuse spheres for the mirror to reflect
    scene.addSphere(Sphere(vec3(-3.0f, 0.5f, -5.0f), 0.5f, vec3(1.0f, 0.2f, 0.2f)));// Red
    scene.addSphere(Sphere(vec3(3.0f, 0.5f, -7.0f), 0.5f, vec3(0.2f, 0.2f, 1.0f)));// Blue
    scene.addSphere(Sphere(vec3(1.5f, 0.4f, -3.5f), 0.4f, vec3(0.2f, 1.0f, 0.2f)));// Green

    // vec3 top(0.0f, 4.5f, -8.0f);// +Y
    // vec3 bottom(0.0f, 1.5f, -5.0f);// -Y
    // vec3 front(0.0f, 3.0f, -3.5f);// +Z (closer)
    // vec3 back(0.0f, 3.0f, -6.5f);// -Z (farther)
    // vec3 right(1.5f, 3.0f, -5.0f);// +X
    // vec3 left(-1.5f, 3.0f, -5.0f);// -X
    //
    //
    // // Upper 4 faces (connecting top vertex to equatorial edges)
    // scene.addTriangle(Triangle(top, front, right, Material::Mirror()));
    // scene.addTriangle(Triangle(top, right, back, Material::Mirror()));
    // scene.addTriangle(Triangle(top, back, left, Material::Mirror()));
    // scene.addTriangle(Triangle(top, left, front, Material::Mirror()));
    //
    // // Lower 4 faces (connecting bottom vertex to equatorial edges)
    // scene.addTriangle(Triangle(bottom, right, front, Material::Mirror()));
    // scene.addTriangle(Triangle(bottom, back, right, Material::Mirror()));
    // scene.addTriangle(Triangle(bottom, left, back, Material::Mirror()));
    // scene.addTriangle(Triangle(bottom, front, left, Material::Mirror()));
    // Camera angled down to see the plane stretching out
    scene.setCamera(std::make_shared<PerspectiveBasicCamera>(
      vec3(0.0f, 4.0f, 2.0f), vec3(0.0f, -0.4f, -1.0f), 2.0f));

    // Lights
    scene.addLight(Light(vec3(5.0f, 8.0f, 2.0f), vec3(1.0f, 1.0f, 1.0f)));
    scene.addLight(Light(vec3(-5.0f, 6.0f, -3.0f), vec3(0.5f, 0.5f, 0.5f)));

    return scene;
  }

  // Create a hexagonal-tiled floor with a mirror sphere above it
  static Scene createHexboardScene()
  {
    Scene scene;

    // Hex-tiled floor at y=0, normal pointing up
    Material black(vec3(0.05f, 0.05f, 0.05f));
    Material white(vec3(0.95f, 0.95f, 0.95f));
    scene.addPlane(Plane(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), black, white, 2.0f, Plane::HEX));

    // Large mirror sphere floating above the plane
    scene.addSphere(Sphere(vec3(0.0f, 1.5f, -6.0f), 1.5f, Material::Mirror()));

    // Small colored diffuse spheres for the mirror to reflect
    scene.addSphere(Sphere(vec3(-3.0f, 0.5f, -5.0f), 0.5f, vec3(1.0f, 0.2f, 0.2f)));// Red
    scene.addSphere(Sphere(vec3(3.0f, 0.5f, -7.0f), 0.5f, vec3(0.2f, 0.2f, 1.0f)));// Blue
    scene.addSphere(Sphere(vec3(1.5f, 0.4f, -3.5f), 0.4f, vec3(0.2f, 1.0f, 0.2f)));// Green

    // Camera angled down to see the plane stretching out
    scene.setCamera(std::make_shared<PerspectiveBasicCamera>(
      vec3(0.0f, 4.0f, 2.0f), vec3(0.0f, -0.4f, -1.0f), 2.0f));

    // Lights
    scene.addLight(Light(vec3(5.0f, 8.0f, 2.0f), vec3(1.0f, 1.0f, 1.0f)));
    scene.addLight(Light(vec3(-5.0f, 6.0f, -3.0f), vec3(0.5f, 0.5f, 0.5f)));

    return scene;
  }


  static Scene createDodecahedronScene()
  {
    Scene scene;

    float sUniform = 1.5f / std::sqrt(3.0f);

    scene.addSphere(Sphere(vec3(4.0f, 0.0f, -8.0f), 1.0f, vec3(1.0f, 1.0f, 1.0f)));

    scene.addSolid(Solid::Dodecahedron(), vec3(0.0f, 0.0f, -8.0f), sUniform, Material(vec3(1.0f, 1.0f, 1.0f)));

    // Camera: pulled back and elevated to see all five solids
    scene.setCamera(std::make_shared<PerspectiveBasicCamera>(
      vec3(7.5f, 5.0f, 0.0f), vec3(-0.7f, -0.6f, -1.0f), 2.5f));

    // Lights
    scene.addLight(Light(vec3(10.0f, 0.0f, 0.0f), vec3(1.0f, 0.0f, 0.0f)));
    scene.addLight(Light(vec3(0.0f, -10.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f)));
    scene.addLight(Light(vec3(0.0, 0.0f, 0.0f), vec3(0.0, 0.0f, 1.0f)));

    return scene;
  }

  // Create a scene displaying all five Platonic solids
  static Scene createPlatonicSolidsScene()
  {
    Scene scene;

    const float phi = (1.0f + std::sqrt(5.0f)) / 2.0f;

    Material floorWhite(vec3(0.9f, 0.9f, 0.9f));
    scene.addPlane(Plane(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), floorWhite));

    // Shared scale: normalize each solid so circumradius ≈ 1.5
    float sUniform = 1.5f / std::sqrt(3.0f);// for tet, cube, dodec (circumR = √3)
    float sOcta = 1.5f;// circumR = 1
    float sIcosa = 1.5f / std::sqrt(2.0f + phi);// circumR = √(2+φ)

    float y = 1.8f;// center height above floor

    scene.addSolid(Solid::Tetrahedron(), vec3(-7.0f, y, -8.0f), sUniform, Material(vec3(1.0f, 0.2f, 0.2f)));
    scene.addSolid(Solid::Cube(), vec3(-3.5f, y, -8.0f), sUniform, Material(vec3(0.2f, 0.4f, 1.0f)));
    scene.addSolid(Solid::Octahedron(), vec3(0.0f, y, -8.0f), sOcta, Material(vec3(1.0f, 0.84f, 0.0f)));
    scene.addSolid(Solid::Dodecahedron(), vec3(3.5f, y, -8.0f), sUniform, Material(vec3(1.0f, 0.6f, 0.2f)));
    scene.addSolid(Solid::Icosahedron(), vec3(7.0f, y, -8.0f), sIcosa, Material(vec3(0.2f, 1.0f, 0.3f)));

    // Camera: pulled back and elevated to see all five solids
    scene.setCamera(std::make_shared<PerspectiveBasicCamera>(
      vec3(0.0f, 4.0f, 0.0f), vec3(0.0f, -0.35f, -1.0f), 2.0f));

    // Lights
    scene.addLight(Light(vec3(5.0f, 10.0f, 5.0f), vec3(0.5f, 0.5f, 0.5f)));
    scene.addLight(Light(vec3(-8.0f, 8.0f, -20.0f), vec3(0.7f, 0.7f, 0.7f)));

    return scene;
  }

  // Create a scene demonstrating per-object shader assignment:
  // Three spheres and a dodecahedron, each forced to a different shading algorithm,
  // regardless of the global --shader flag.
  static Scene createMixedShaderScene(const SceneParams &params = {})
  {
    Scene scene;

    const float mirror_sphere_height = params.count("mirror_sphere_height") ? params.at("mirror_sphere_height") : 5.0f;

    scene.setBackgroundColor(vec3(0.5f, 0.7f, 1.0f));

    // Left sphere
    scene.addSphere(Sphere(vec3(-4.5f, 0.0f, -4.0f), 1.0f, Material(vec3(0.9f, 0.9f, 0.9f), ShaderType::MIRROR)));

    // Center dodecahedron — diffuse shading
    scene.addSolid(Solid::Dodecahedron(), vec3(0.0f, 0.0f, -2.0f), 1.0f / std::sqrt(3.0f), Material(vec3(0.9f, 0.3f, 0.0f), ShaderType::DIFFUSE));

    // Right sphere
    scene.addSphere(Sphere(vec3(4.5f, 0.0f, -4.0f), 1.0f, Material(vec3(0.3f, 0.5f, 1.0f), ShaderType::BLINN_PHONG)));

    // Above sphere
    scene.addSphere(Sphere(vec3(0.0f, mirror_sphere_height, -6.0f), 3.0f, Material(vec3(0.9f, 0.9f, 0.9f), ShaderType::MIRROR)));

    Material floorLight(vec3(0.1f, 0.4f, 0.1f));
    scene.addPlane(Plane(vec3(0.0f, -1.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), floorLight));

    // Pull camera back to keep all four spheres in frame
    scene.setCamera(std::make_shared<PerspectiveBasicCamera>(
      vec3(0.0f, 2.2f, 2.5f), vec3(0.0f, 0.0f, -1.0f), 1.0f));

    // Two lights to make shading differences visible
    scene.addLight(Light(vec3(5.0f, 5.0f, 0.0f), vec3(1.0f, 1.0f, 1.0f)));
    scene.addLight(Light(vec3(-3.0f, 2.0f, 2.0f), vec3(0.4f, 0.4f, 0.4f)));

    return scene;
  }

  // A single sphere sitting on a plane, lit by three light sources
  // from different positions so each casts a distinct shadow onto the plane.
  static Scene createShadowDemoScene(const SceneParams &params = {})
  {
    Scene scene;

    const float t = params.count("t") ? params.at("t") : 1.0f;

    float s0, s1, s2;

    // Compute light intensity scalars
    if (t <= 0.333f) {
      s0 = 3 * t;
      s1 = 0;
      s2 = 0;
    } else if (t > 0.333f and t <= 0.666f) {
      s0 = 1;
      s1 = 3 * (t - 0.333f);
      s2 = 0;
    } else {
      s0 = 1;
      s1 = 1;
      s2 = 3 * (t - 0.666f);
    }

    Material floorLight(vec3(0.85f, 0.85f, 0.85f));
    scene.addPlane(Plane(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), floorLight));

    // Blue sphere sitting on the plane (center at y = radius = 1)
    scene.addSphere(Sphere(vec3(0.0f, 1.0f, -5.0f), 1.0f, Material(vec3(0.0f, 0.0f, 1.0f))));

    // Camera angled down to see both the sphere and the shadows spreading on the floor
    scene.setCamera(std::make_shared<PerspectiveBasicCamera>(
      vec3(0.0f, 5.0f, 2.0f), vec3(0.0f, -0.6f, -1.0f), 2.5f));

    // Three lights at different azimuths — each casts a shadow in a different direction
    scene.addLight(Light(vec3(-5.0f, 6.0f, -2.0f), s0 * vec3(0.5f, 0.5f, 0.5f)));
    scene.addLight(Light(vec3(5.0f, 6.0f, -2.0f), s1 * vec3(0.5f, 0.5f, 0.5f)));
    scene.addLight(Light(vec3(0.0f, 7.0f, -10.0f), s2 * vec3(0.5f, 0.5f, 0.5f)));

    return scene;
  }

  // Test Triangle image
  static Scene createTriangleTestScene()
  {
    Scene scene;

    scene.addTriangle(Triangle(
      vec3(-1.2f, -0.2f, -7.0f),
      vec3(0.8f, -0.5f, -5.0f),
      vec3(0.9, 0.0, -5.0f),
      vec3(1.0f, 0.0f, 0.0f)));

    scene.addTriangle(Triangle(
      vec3(0.773205f, -0.93923f, -7.0f),
      vec3(0.0330127, 0.94282, -5.0f),
      vec3(-0.45f, 0.779423f, -5.0f),
      vec3(0.0f, 1.0f, 0.0f)));

    scene.addTriangle(Triangle(
      vec3(0.426795f, 1.13923f, -7.0f),
      vec3(-0.833013f, -0.44282f, -5.0f),
      vec3(-0.45f, -0.779423f, -5.0f),
      vec3(0.0f, 0.0f, 1.0f)));

    scene.setCamera(std::make_shared<PerspectiveBasicCamera>(
      vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f), 4.0f));

    scene.addLight(Light(vec3(5.0f, 8.0f, 2.0f), vec3(1.0f, 1.0f, 1.0f)));

    return scene;
  }

  static Scene createHallOfMirrorsScene()
  {
    Scene scene;

    // Central spheres
    // scene.addSphere(Sphere(vec3(0.0f, 0.0f, -10.0f), 1.0f, Material(vec3(0.9f, 0.9f, 0.9f), ShaderType::BLINN_PHONG)));

    vec3 color1 = HSLColor(180.0f, 1.0f, 0.5f).toRgb();

    // scene.addSphere(Sphere(vec3(0.0f, 0.0f, 0.0f), 0.5f, Material(color1, ShaderType::BLINN_PHONG)));

    vec3 color2 = HSLColor(270.0f, 1.0f, 0.5f).toRgb();

    scene.addSphere(Sphere(vec3(0.0f, 1.0f, 0.0f), 0.25f, Material(color2, ShaderType::BLINN_PHONG)));

    vec3 color3 = HSLColor(90.0f, 1.0f, 0.5f).toRgb();

    scene.addSphere(Sphere(vec3(0.0f, -1.0f, 0.0), 0.25f, Material(color3, ShaderType::BLINN_PHONG)));

    vec3 color4 = HSLColor(360.0f, 1.0f, 0.5f).toRgb();

    scene.addSphere(Sphere(vec3(1.0f, 0.0f, 0.0), 0.25f, Material(color4, ShaderType::BLINN_PHONG)));

    vec3 color5 = HSLColor(45.0f, 1.0f, 0.5f).toRgb();

    scene.addSphere(Sphere(vec3(-1.0f, 0.0f, 0.0), 0.25f, Material(color5, ShaderType::BLINN_PHONG)));

    vec3 color6 = HSLColor(135.0f, 1.0f, 0.5f).toRgb();

    scene.addSphere(Sphere(vec3(0.0f, 0.0f, 0.5f), 0.25f, Material(color6, ShaderType::BLINN_PHONG)));

    vec3 color7 = HSLColor(225.0f, 1.0f, 0.5f).toRgb();

    scene.addSphere(Sphere(vec3(0.0f, 0.0f, -0.5f), 0.25f, Material(color7, ShaderType::BLINN_PHONG)));


    Material mirrorMaterial(vec3(0.9f, 0.9f, 0.9f), ShaderType::MIRROR);

    scene.addSphere(Sphere(vec3(-10.0f, 0.0f, 0.0f), 5.0f, mirrorMaterial));

    scene.addSphere(Sphere(vec3(10.0f, 0.0f, 0.0f), 5.0f, mirrorMaterial));

    scene.addSphere(Sphere(vec3(0.0f, -10.0f, 0.0f), 5.0f, mirrorMaterial));

    scene.addSphere(Sphere(vec3(0.0f, 10.0f, 0.0f), 5.0f, mirrorMaterial));

    scene.addSphere(Sphere(vec3(0.0f, 0.0f, -10.0f), 5.0f, mirrorMaterial));

    scene.addSphere(Sphere(vec3(0.0f, 0.0f, 10.0f), 5.0f, mirrorMaterial));
    // Mirror planes
    // scene.addPlane(Plane(vec3(-2.0f, 0.0f, 0.0f), vec3(1.0f, 0.0f, 0.0f), mirrorMaterial));
    //
    // scene.addPlane(Plane(vec3(2.0f, 0.0f, 0.0f), vec3(-1.0f, 0.0f, 0.0f), mirrorMaterial));

    // Pull camera back to keep all four spheres in frame
    scene.setCamera(std::make_shared<PerspectiveBasicCamera>(
      vec3(0.0f, 0.0f, 1.0f), vec3(0.0f, 0.0f, -1.0f), 0.25f));

    // front light
    scene.addLight(Light(vec3(0.0f, 0.0f, 1.0f), vec3(0.5f, 0.5f, 0.5f)));
    // back light
    scene.addLight(Light(vec3(0.0f, 0.0f, -1.0f), vec3(0.5f, 0.5f, 0.5f)));

    return scene;
  }
  // Random spheres of varying size and hue sitting on a white plane.
  // Params:
  //   number_spheres — how many spheres to place (default 20)
  //   seed           — random seed for reproducibility (default 42)
  static Scene createRandomSpheresScene(const SceneParams &params = {})
  {
    Scene scene;

    const int numSpheres = params.count("number_spheres")
                             ? static_cast<int>(params.at("number_spheres"))
                             : 20;

    const uint32_t seed = params.count("seed")
                            ? static_cast<uint32_t>(params.at("seed"))
                            : 42;

    // White floor
    Material floorWhite(vec3(0.95f, 0.95f, 0.95f));
    scene.addPlane(Plane(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), floorWhite));

    // Vertical back wall
    Material wallWhite(vec3(0.90f, 0.90f, 0.90f));
    scene.addPlane(Plane(vec3(0.0f, 0.0f, -13.0f), vec3(0.0f, 0.0f, 1.0f), wallWhite));

    // RNG with configurable seed for reproducibility
    std::mt19937 rng(seed);
    std::uniform_real_distribution<float> radiusDist(0.3f, 1.0f);
    std::uniform_real_distribution<float> hueDist(0.0f, 360.0f);
    std::uniform_real_distribution<float> posDist(-8.0f, 8.0f);
    std::uniform_real_distribution<float> depthDist(-12.0f, -2.0f);

    // Track placed spheres (center on xz-plane + radius) for overlap rejection
    struct Placed
    {
      float x, z, r;
    };
    std::vector<Placed> placed;
    placed.reserve(numSpheres);

    const int maxAttempts = 200;// per sphere before giving up
    for (int i = 0; i < numSpheres; ++i) {
      bool ok = false;
      for (int attempt = 0; attempt < maxAttempts; ++attempt) {
        float radius = radiusDist(rng);
        float x = posDist(rng);
        float z = depthDist(rng);

        // Check against all previously placed spheres (xz distance vs sum of radii)
        bool overlaps = false;
        for (const auto &p : placed) {
          float dx = x - p.x;
          float dz = z - p.z;
          if (dx * dx + dz * dz < (radius + p.r) * (radius + p.r)) {
            overlaps = true;
            break;
          }
        }
        if (overlaps) continue;

        placed.push_back({ x, z, radius });
        // Every 1-in-6 sphere is a mirror
        if (i % 6 == 0) {
          scene.addSphere(Sphere(vec3(x, radius, z), radius, Material::Mirror()));
        } else {
          vec3 color = HSLColor(hueDist(rng), 1.0f, 0.5f).toRgb();
          scene.addSphere(Sphere(vec3(x, radius, z), radius, Material(color)));
        }
        ok = true;
        break;
      }
      if (!ok) break;// couldn't place any more without overlap
    }

    // Camera and lights copied from shadow demo
    scene.setCamera(std::make_shared<PerspectiveBasicCamera>(
      vec3(-2.0f, 5.0f, 2.0f), vec3(.1f, -0.5f, -1.0f), 1.75f));

    scene.addLight(Light(vec3(-5.0f, 6.0f, -2.0f), vec3(0.1f, 0.1f, 0.1f)));
    scene.addLight(Light(vec3(5.0f, 6.0f, -2.0f), vec3(0.3f, 0.3f, 0.3f)));
    scene.addLight(Light(vec3(0.0f, 7.0f, -10.0f), vec3(0.2f, 0.2f, 0.2f)));

    return scene;
  }

  // Create a scene from a triangle list data file.
  // The file contains comma-separated floats: every 9 floats define one triangle
  // (3 vertices x 3 components each).
  static Scene createTrilistScene(const std::string &filepath)
  {
    Scene scene;

    std::ifstream file(filepath);
    if (!file.is_open()) {
      throw std::runtime_error("Cannot open trilist file: " + filepath);
    }

    // Parse all floats from the file (comma-separated)
    std::vector<float> values;
    std::string token;
    while (std::getline(file, token, ',')) {
      values.push_back(std::stof(token));
    }

    if (values.size() % 9 != 0) {
      throw std::runtime_error("Trilist file must contain a multiple of 9 floats, got " + std::to_string(values.size()));
    }

    // Build triangles and track bounding box
    vec3 bmin(1e30f, 1e30f, 1e30f);
    vec3 bmax(-1e30f, -1e30f, -1e30f);

    Material mat(vec3(0.8f, 0.8f, 0.8f));
    for (size_t i = 0; i < values.size(); i += 9) {
      vec3 v0(values[i + 0], values[i + 1], values[i + 2]);
      vec3 v1(values[i + 3], values[i + 4], values[i + 5]);
      vec3 v2(values[i + 6], values[i + 7], values[i + 8]);
      scene.addTriangle(Triangle(v0, v1, v2, mat));

      for (int c = 0; c < 3; ++c) {
        bmin[c] = std::min({ bmin[c], v0[c], v1[c], v2[c] });
        bmax[c] = std::max({ bmax[c], v0[c], v1[c], v2[c] });
      }
    }

    // Auto-fit camera: look at the center, pull back far enough to see everything
    vec3 center = (bmin + bmax) * 0.5f;
    vec3 extent = bmax - bmin;
    float maxExtent = std::max({ extent[0], extent[1], extent[2] });

    // Position camera in front of the object (positive Z from center)
    vec3 camPos = center + vec3(0.0f, 0.0f, maxExtent * 1.5f);
    vec3 camDir = (center - camPos).normalized();
    scene.setCamera(std::make_shared<PerspectiveBasicCamera>(camPos, camDir, 2.0f * maxExtent * 0.6f));

    // Lights above and to the sides
    scene.addLight(Light(center + vec3(maxExtent, maxExtent, maxExtent), vec3(1.0f, 1.0f, 1.0f)));
    scene.addLight(Light(center + vec3(-maxExtent, maxExtent * 0.5f, maxExtent), vec3(0.5f, 0.5f, 0.5f)));

    return scene;
  }

  // Dispatch a scene by preset name. Centralizes the string→factory mapping so
  // both the raytracer and the OpenGL entry point can use the same names.
  static Scene loadScene(const std::string &preset_name,
                         const SceneParams &params = {},
                         const std::string &datafile = "")
  {
    if (preset_name == "trilist") {
      if (datafile.empty()) {
        throw std::invalid_argument("The 'trilist' preset requires --datafile <path>");
      }
      return createTrilistScene(datafile);
    } else if (preset_name == "test") {
      return createTestScene();
    } else if (preset_name == "lit_test") {
      return createLitTestScene();
    } else if (preset_name == "single_sphere") {
      return createSingleSphereScene();
    } else if (preset_name == "grid") {
      return createGridScene();
    } else if (preset_name == "aligned") {
      return createAlignedSpheresScene();
    } else if (preset_name == "spiral") {
      return createSpiralSpheresScene(params);
    } else if (preset_name == "pyramid") {
      return createTrianglePyramidScene();
    } else if (preset_name == "octahedron") {
      return createOctahedronScene();
    } else if (preset_name == "dodecahedron") {
      return createDodecahedronScene();
    } else if (preset_name == "checkerboard") {
      return createCheckerboardScene();
    } else if (preset_name == "hexboard") {
      return createHexboardScene();
    } else if (preset_name == "platonic") {
      return createPlatonicSolidsScene();
    } else if (preset_name == "triangle_test") {
      return createTriangleTestScene();
    } else if (preset_name == "mixed_shader") {
      return createMixedShaderScene(params);
    } else if (preset_name == "shadow_demo") {
      return createShadowDemoScene(params);
    } else if (preset_name == "hall_of_mirrors") {
      return createHallOfMirrorsScene();
    } else if (preset_name == "random_spheres") {
      return createRandomSpheresScene(params);
    } else if (preset_name == "fresnel_caustic") {
      return createFresnelCausticScene(params);
    } else {
      throw std::invalid_argument("Unknown scene preset: " + preset_name);
    }
  }

  // Fresnel + caustics demo scene.
  // Z-up convention: light high on +Z, water surface centered on the XY-plane with
  // Perlin-noise displacement in Z, matte floor far below on -Z. Camera in upper +Z
  // pointing toward the floor. Photons traced from the light, refracted through
  // the water, accumulate as caustics on the floor.
  //
  // Params:
  //   light_height       — z position of the point light (default 8.0)
  //   light_intensity    — scalar brightness (default 50.0; high because most flux refracts)
  //   water_size         — full extent of the water surface in X and Y (default 8.0)
  //   water_resolution   — grid divisions per side; produces 2*N^2 triangles (default 64)
  //   water_amplitude    — Perlin Z displacement amplitude (default 0.15)
  //   water_frequency    — Perlin sampling frequency (default 1.5)
  //   water_z            — base z height of the water (default 0.0)
  //   floor_z            — z height of the matte floor (default -5.0)
  //   seed               — Perlin seed (default 42)
  //   cam_height         — z position of the camera (default 6.0)
  //   cam_back           — y offset of the camera (default 3.0; positions the camera back along -Y)
  //   ball_height        — z position of the submerged diffuse ball's center (default -1.0; below water_z)
  //   ball_radius        — radius of the diffuse ball (default 0.8)
  //   ball_color_r/g/b   — RGB color of the diffuse ball (defaults: 1.0, 0.45, 0.15 — warm orange)
  static Scene createFresnelCausticScene(const SceneParams &params = {})
  {
    auto P = [&](const std::string &k, float def) {
      auto it = params.find(k);
      return it == params.end() ? def : it->second;
    };

    Scene scene;

    const float light_height    = P("light_height", 8.0f);
    const float light_intensity = P("light_intensity", 50.0f);
    const float water_size      = P("water_size", 8.0f);
    const int   N               = static_cast<int>(P("water_resolution", 64.0f));
    const float water_amplitude = P("water_amplitude", 0.15f);
    const float water_frequency = P("water_frequency", 1.5f);
    const float water_z         = P("water_z", 0.0f);
    const float floor_z         = P("floor_z", -5.0f);
    const uint32_t seed         = static_cast<uint32_t>(P("seed", 42.0f));
    const float cam_height      = P("cam_height", 6.0f);
    const float cam_back        = P("cam_back", 3.0f);
    const float ball_height     = P("ball_height", -1.0f);
    const float ball_radius     = P("ball_radius", 0.8f);
    const vec3  ball_color(P("ball_color_r", 1.0f),
                           P("ball_color_g", 0.45f),
                           P("ball_color_b", 0.15f));

    // Background: dim sky so reflected component on the water reads as "darker" rather than zero.
    scene.setBackgroundColor(vec3(0.02f, 0.04f, 0.08f));

    // Point light high above the water.
    scene.addLight(Light(vec3(0.0f, 0.0f, light_height),
                         vec3(light_intensity, light_intensity, light_intensity)));

    // Generate a Perlin-displaced height field on an (N+1) x (N+1) vertex grid.
    PerlinNoise pn(seed);
    const float half = water_size * 0.5f;
    const float dx = water_size / static_cast<float>(N);
    const float dy = water_size / static_cast<float>(N);

    std::vector<std::vector<float>> heights(N + 1, std::vector<float>(N + 1, 0.0f));
    for (int i = 0; i <= N; ++i) {
      for (int j = 0; j <= N; ++j) {
        float x = -half + dx * static_cast<float>(i);
        float y = -half + dy * static_cast<float>(j);
        heights[i][j] = water_z + water_amplitude *
                        pn.noise2D(x * water_frequency, y * water_frequency);
      }
    }

    // Vertex positions.
    std::vector<std::vector<vec3>> verts(N + 1, std::vector<vec3>(N + 1));
    for (int i = 0; i <= N; ++i) {
      for (int j = 0; j <= N; ++j) {
        float x = -half + dx * static_cast<float>(i);
        float y = -half + dy * static_cast<float>(j);
        verts[i][j] = vec3(x, y, heights[i][j]);
      }
    }

    // Per-vertex normals from the analytical heightfield gradient: n = (-dh/dx, -dh/dy, 1).
    std::vector<std::vector<vec3>> normals(N + 1, std::vector<vec3>(N + 1));
    for (int i = 0; i <= N; ++i) {
      for (int j = 0; j <= N; ++j) {
        float dh_dx, dh_dy;
        if (i == 0)        dh_dx = (heights[i + 1][j] - heights[i][j]) / dx;
        else if (i == N)   dh_dx = (heights[i][j] - heights[i - 1][j]) / dx;
        else               dh_dx = (heights[i + 1][j] - heights[i - 1][j]) / (2.0f * dx);
        if (j == 0)        dh_dy = (heights[i][j + 1] - heights[i][j]) / dy;
        else if (j == N)   dh_dy = (heights[i][j] - heights[i][j - 1]) / dy;
        else               dh_dy = (heights[i][j + 1] - heights[i][j - 1]) / (2.0f * dy);
        normals[i][j] = vec3(-dh_dx, -dh_dy, 1.0f).normalized();
      }
    }

    // Build triangles. Winding (v0, v1, v2) such that face normal points +Z.
    Material water = Material::Water();
    for (int i = 0; i < N; ++i) {
      for (int j = 0; j < N; ++j) {
        // Triangle 1: (i,j) -> (i+1,j) -> (i+1,j+1)
        scene.addTriangle(Triangle(
          verts[i][j],   verts[i + 1][j],   verts[i + 1][j + 1],
          normals[i][j], normals[i + 1][j], normals[i + 1][j + 1],
          water));
        // Triangle 2: (i,j) -> (i+1,j+1) -> (i,j+1)
        scene.addTriangle(Triangle(
          verts[i][j],   verts[i + 1][j + 1],   verts[i][j + 1],
          normals[i][j], normals[i + 1][j + 1], normals[i][j + 1],
          water));
      }
    }

    // Matte floor at z = floor_z, normal +Z, marked as caustic receiver.
    Material floor_mat(vec3(0.85f, 0.85f, 0.85f), ShaderType::DIFFUSE);
    floor_mat.is_caustic_receiver = true;
    scene.addPlane(Plane(vec3(0.0f, 0.0f, floor_z),
                         vec3(0.0f, 0.0f, 1.0f),
                         floor_mat));

    // Submerged colored diffuse ball — also receives caustics so refracted
    // light dapples its top surface the way it does the floor.
    Material ball_mat(ball_color, ShaderType::DIFFUSE);
    ball_mat.is_caustic_receiver = true;
    scene.addSphere(Sphere(vec3(0.0f, 0.0f, ball_height), ball_radius, ball_mat));

    // Camera: pulled back along -Y, elevated on +Z, looking toward origin/floor.
    vec3 cam_pos(0.0f, -cam_back, cam_height);
    vec3 cam_target(0.0f, 0.0f, floor_z * 0.5f);
    vec3 cam_dir = (cam_target - cam_pos).normalized();
    scene.setCamera(std::make_shared<PerspectiveBasicCamera>(cam_pos, cam_dir, 1.5f));

    return scene;
  }
};

#endif// SCENEPRESETS_H
