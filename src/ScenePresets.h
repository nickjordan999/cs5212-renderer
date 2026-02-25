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
#include "vec.h"
#include <memory>
#include <random>
#include <cmath>
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
    const int numSpheres = params.count("num_spheres") ? static_cast<int>(params.at("num_spheres")) : 500;
    const float radius = 1.0f;
    const float spacing = 2.0f * radius;
    const float spiralRadius = 5.0f;
    const float spiralTurns = params.count("spiral_turns") ? params.at("spiral_turns") : 75.0f;

    for (int i = 0; i < numSpheres; ++i) {
      // Position along z-axis
      float z = -i * spacing;

      // Calculate angle for spiral (increases with each sphere)
      float angle = (i / static_cast<float>(numSpheres)) * 2.0f * 3.14159265359f * spiralTurns;

      // Position on spiral
      float x = spiralRadius * std::cos(angle);
      float y = spiralRadius * std::sin(angle);

      float hue_shift = params.count("hue_shift") ? params.at("hue_shift") : 0.0f;// Cycle hue across all spheres, max saturation and lightness
      float hue = std::fmod((i / static_cast<float>(numSpheres)) * 360.0f + hue_shift, 360.f);
      vec3 color = HSLColor(hue, 1.0f, 0.5f).toRgb();

      scene.addSphere(Sphere(vec3(x, y, z), radius, Material(color)));
    }

    // Position camera along the spiral's z-extent based on parameter t
    float t = params.count("t") ? params.at("t") : -0.1f;
    float totalZ = (numSpheres - 1) * spacing;
    scene.setCamera(std::make_shared<PerspectiveBasicCamera>(
      vec3(0.0f, 0.0f, -t * totalZ), vec3(0.0f, 0.0f, -1.0f), 2.0f));

    scene.addLight(Light(vec3(0.0f, 0.0f, -totalZ / 4.0f), vec3(1.0f, 1.0f, 1.0f)));

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

    // Checkerboard floor
    Material floorBlack(vec3(0.1f, 0.1f, 0.1f));
    Material floorWhite(vec3(0.9f, 0.9f, 0.9f));
    scene.addPlane(Plane(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), floorBlack, floorWhite, 2.0f));

    // Shared scale: normalize each solid so circumradius ≈ 1.5
    float sUniform = 1.5f / std::sqrt(3.0f);// for tet, cube, dodec (circumR = √3)
    float sOcta = 1.5f;// circumR = 1
    float sIcosa = 1.5f / std::sqrt(2.0f + phi);// circumR = √(2+φ)

    float y = 1.8f;// center height above floor

    scene.addSolid(Solid::Tetrahedron(), vec3(-7.0f, y, -8.0f), sUniform, Material(vec3(1.0f, 0.2f, 0.2f)));
    scene.addSolid(Solid::Cube(), vec3(-3.5f, y, -8.0f), sUniform, Material(vec3(0.2f, 0.4f, 1.0f)));
    scene.addSolid(Solid::Octahedron(), vec3(0.0f, y, -8.0f), sOcta, Material(vec3(1.0f, 0.84f, 0.0f)));
    scene.addSolid(Solid::Icosahedron(), vec3(7.0f, y, -8.0f), sIcosa, Material(vec3(0.2f, 1.0f, 0.3f)));

    // Camera: pulled back and elevated to see all five solids
    scene.setCamera(std::make_shared<PerspectiveBasicCamera>(
      vec3(0.0f, 4.0f, 0.0f), vec3(0.0f, -0.35f, -1.0f), 1.0f));

    // Lights
    scene.addLight(Light(vec3(5.0f, 10.0f, 5.0f), vec3(1.0f, 1.0f, 1.0f)));
    scene.addLight(Light(vec3(-8.0f, 8.0f, 0.0f), vec3(0.5f, 0.5f, 0.5f)));

    return scene;
  }

  // Create a scene demonstrating per-object shader assignment:
  // Three spheres and a dodecahedron, each forced to a different shading algorithm,
  // regardless of the global --shader flag.
  static Scene createMixedShaderScene()
  {
    Scene scene;

    scene.setBackgroundColor(vec3(0.5f, 0.7f, 1.0f));

    // Left sphere
    scene.addSphere(Sphere(vec3(-4.5f, 0.0f, -4.0f), 1.0f, Material(vec3(0.9f, 0.9f, 0.9f), ShaderType::MIRROR)));

    // Center dodecahedron — diffuse shading
    scene.addSolid(Solid::Dodecahedron(), vec3(0.0f, 0.0f, -2.0f), 1.0f / std::sqrt(3.0f), Material(vec3(0.9f, 0.3f, 0.0f), ShaderType::DIFFUSE));

    // Right sphere
    scene.addSphere(Sphere(vec3(4.5f, 0.0f, -4.0f), 1.0f, Material(vec3(0.3f, 0.5f, 1.0f), ShaderType::BLINN_PHONG)));

    // Above sphere
    scene.addSphere(Sphere(vec3(0.0f, 5.0f, -6.0f), 3.0f, Material(vec3(0.9f, 0.9f, 0.9f), ShaderType::MIRROR)));

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

  // A single sphere sitting on a checkerboard plane, lit by three light sources
  // from different positions so each casts a distinct shadow onto the plane.
  static Scene createShadowDemoScene()
  {
    Scene scene;

    // Checkerboard floor — high contrast so shadows are clearly visible
    Material floorLight(vec3(0.85f, 0.85f, 0.85f));
    Material floorDark(vec3(0.2f, 0.2f, 0.2f));
    scene.addPlane(Plane(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), floorLight, floorDark, 2.0f));

    // Orange sphere sitting on the plane (center at y = radius = 1)
    scene.addSphere(Sphere(vec3(0.0f, 1.0f, -5.0f), 1.0f, Material(vec3(0.9f, 0.5f, 0.1f))));

    // Camera angled down to see both the sphere and the shadows spreading on the floor
    scene.setCamera(std::make_shared<PerspectiveBasicCamera>(
      vec3(0.0f, 5.0f, 2.0f), vec3(0.0f, -0.5f, -1.0f), 2.0f));

    // Three lights at different azimuths — each casts a shadow in a different direction
    scene.addLight(Light(vec3(-5.0f, 6.0f, -2.0f), vec3(1.0f, 0.85f, 0.7f)));// left-front, warm
    scene.addLight(Light(vec3(5.0f, 6.0f, -2.0f), vec3(0.7f, 0.85f, 1.0f)));// right-front, cool
    scene.addLight(Light(vec3(0.0f, 7.0f, -10.0f), vec3(0.8f, 1.0f, 0.8f)));// rear, neutral green-white

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
    scene.addLight(Light(vec3(0.0f, 0.0f, 1.0f), vec3(1.0f, 1.0f, 1.0f)));
    // back light
    scene.addLight(Light(vec3(0.0f, 0.0f, -1.0f), vec3(1.0f, 1.0f, 1.0f)));

    return scene;
  }
};

#endif// SCENEPRESETS_H
