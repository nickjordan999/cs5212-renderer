#include "Shader.h"
#include "HitRecord.h"
#include <cmath>
#include <thread>
#include <vector>

// --- Free helper functions for shading computations ---

static vec3 computeDiffuse(const Scene &scene, const HitRecord &hit, bool shadows)
{
  vec3 shaded_color(0.0f, 0.0f, 0.0f);
  const auto &lights = scene.getLights();
  for (const auto &light : lights) {
    vec3 to_light = light.getPosition() - hit.point;
    float light_dist = to_light.length();
    vec3 light_dir = to_light / light_dist;

    if (shadows && scene.isOccluded(Ray(hit.point, light_dir), light_dist)) continue;

    float diffuse_factor = std::max(0.0f, hit.normal.dot(light_dir));
    shaded_color = shaded_color + (hit.material.color * light.getIntensity() * diffuse_factor);
  }
  vec3 ambient(0.1f, 0.1f, 0.1f);
  shaded_color = shaded_color + (hit.material.color * ambient);
  return shaded_color;
}

static vec3 computeBlinnPhong(const Scene &scene, const Ray &ray, const HitRecord &hit, bool shadows)
{
  vec3 view_dir = (ray.getDirection() * -1.0f).normalized();
  vec3 shaded_color(0.0f, 0.0f, 0.0f);
  const auto &lights = scene.getLights();
  for (const auto &light : lights) {
    vec3 to_light = light.getPosition() - hit.point;
    float light_dist = to_light.length();
    vec3 light_dir = to_light / light_dist;

    if (shadows && scene.isOccluded(Ray(hit.point, light_dir), light_dist)) continue;

    float diff = std::max(0.0f, hit.normal.dot(light_dir));
    vec3 halfway = (light_dir + view_dir).normalized();
    float spec = std::pow(std::max(0.0f, hit.normal.dot(halfway)), 32.0f);
    shaded_color = shaded_color
                   + hit.material.color * light.getIntensity() * diff
                   + light.getIntensity() * spec * 0.5f;
  }
  vec3 ambient(0.1f, 0.1f, 0.1f);
  shaded_color = shaded_color + (hit.material.color * ambient);
  return shaded_color;
}

static vec3 computeNormal(const HitRecord &hit)
{
  return vec3(
    (hit.normal[0] + 1.0f) * 0.5f,
    (hit.normal[1] + 1.0f) * 0.5f,
    (hit.normal[2] + 1.0f) * 0.5f);
}

// --- Base Shader ---

void Shader::traceScene(FrameBuffer &fb, int raysPerPixel) const
{
  const Camera &camera = scene.getCamera();
  float w = fb.getWidth();
  float h = fb.getHeight();
  size_t height = static_cast<size_t>(fb.getHeight());
  size_t width = static_cast<size_t>(fb.getWidth());

  unsigned int numThreads = std::thread::hardware_concurrency();
  if (numThreads == 0) numThreads = 4;

  auto renderRows = [&](size_t yStart, size_t yEnd) {
    for (size_t y = yStart; y < yEnd; ++y) {
      for (size_t x = 0; x < width; ++x) {
        vec3 color(0.0f, 0.0f, 0.0f);
        for (int sample = 0; sample < raysPerPixel; ++sample) {
          Ray ray = (raysPerPixel > 1) ? camera.generateRayAA(x, y, w, h) : camera.generateRay(x, y, w, h);
          color = color + shadeRay(ray);
        }
        color = color * (1.0f / static_cast<float>(raysPerPixel));
        fb.setPixel(x, y, color);
      }
    }
  };

  std::vector<std::thread> threads;
  size_t rowsPerThread = height / numThreads;
  size_t remainder = height % numThreads;
  size_t yStart = 0;
  for (unsigned int t = 0; t < numThreads; ++t) {
    size_t yEnd = yStart + rowsPerThread + (t < remainder ? 1 : 0);
    threads.emplace_back(renderRows, yStart, yEnd);
    yStart = yEnd;
  }
  for (auto &thread : threads) {
    thread.join();
  }
}

vec3 Shader::shadeRay(const Ray &ray) const
{
  return shadeRay(ray, maxDepth);
}

vec3 Shader::shadeRay(const Ray &ray, int depth) const
{
  if (depth <= 0) return background;

  auto hit = scene.traceRayWithHitInfo(ray);
  if (!hit.has_value()) return background;

  // Dispatch based on per-object shader override, falling back to this shader's default
  switch (hit->material.shaderType.value_or(defaultShaderType)) {
  case ShaderType::SIMPLE:
    return hit->material.color;
  case ShaderType::NORMAL:
    return computeNormal(*hit);
  case ShaderType::DIFFUSE:
    return computeDiffuse(scene, *hit, shadows);
  case ShaderType::BLINN_PHONG:
    return computeBlinnPhong(scene, ray, *hit, shadows);
  case ShaderType::MIRROR: {
    vec3 reflected_dir = reflect(ray.getDirection().normalized(), hit->normal);
    Ray reflected_ray(hit->point, reflected_dir);
    return hit->material.color * shadeRay(reflected_ray, depth - 1);
  }
  }
  return background;
}
