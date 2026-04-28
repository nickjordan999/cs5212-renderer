#include "Shader.h"
#include "HitRecord.h"
#include "Optics.h"
#include "CausticsMap.h"
#include <cmath>
#include <random>
#include <thread>
#include <vector>

// Thread-local RNG for stochastic sampling (same pattern as Camera)
static thread_local std::mt19937 s_rng(std::random_device{}());
static thread_local std::uniform_real_distribution<float> s_dist(0.0f, 1.0f);

// Cosine-weighted hemisphere sampling using Malley's method.
static vec3 cosineWeightedHemisphere(const vec3 &normal)
{
  float u1 = s_dist(s_rng);
  float u2 = s_dist(s_rng);

  float r = std::sqrt(u1);
  float theta = 2.0f * static_cast<float>(M_PI) * u2;
  float x = r * std::cos(theta);
  float y = r * std::sin(theta);
  float z = std::sqrt(1.0f - u1);

  vec3 w = normal.normalized();
  vec3 a = (std::fabs(w[0]) > 0.9f) ? vec3(0.0f, 1.0f, 0.0f) : vec3(1.0f, 0.0f, 0.0f);
  vec3 u = a.cross(w).normalized();
  vec3 v = w.cross(u);

  return (u * x + v * y + w * z).normalized();
}

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

  unsigned int threadCount = numThreads > 0 ? numThreads : std::thread::hardware_concurrency();
  if (threadCount == 0) threadCount = 4;

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
  size_t rowsPerThread = height / threadCount;
  size_t remainder = height % threadCount;
  size_t yStart = 0;
  for (unsigned int t = 0; t < threadCount; ++t) {
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
  case ShaderType::DIFFUSE: {
    vec3 base = computeDiffuse(scene, *hit, shadows);
    if (causticsMap != nullptr && hit->material.is_caustic_receiver) {
      base = base + causticsMap->estimate(hit->point) * hit->material.color;
    }
    return base;
  }
  case ShaderType::BLINN_PHONG:
    return computeBlinnPhong(scene, ray, *hit, shadows);
  case ShaderType::MIRROR: {
    vec3 reflected_dir = reflect(ray.getDirection().normalized(), hit->normal);
    Ray reflected_ray(hit->point, reflected_dir);
    return hit->material.color * shadeRay(reflected_ray, depth - 1);
  }
  case ShaderType::PATH_DIFFUSE: {
    vec3 direct = computeDiffuse(scene, *hit, shadows);
    if (causticsMap != nullptr && hit->material.is_caustic_receiver) {
      direct = direct + causticsMap->estimate(hit->point) * hit->material.color;
    }
    vec3 bounce_dir = cosineWeightedHemisphere(hit->normal);
    Ray bounce_ray(hit->point, bounce_dir);
    vec3 indirect = hit->material.color * shadeRay(bounce_ray, depth - 1);
    return direct + indirect;
  }
  case ShaderType::DIELECTRIC: {
    vec3 d = ray.getDirection().normalized();
    // Orient the normal against the incoming ray (HitRecord stores the geometric normal).
    vec3 n = hit->is_front_face ? hit->normal : (hit->normal * -1.0f);
    float ior_from = hit->is_front_face ? 1.0f : hit->material.ior;
    float ior_to   = hit->is_front_face ? hit->material.ior : 1.0f;
    float cos_theta = std::min(1.0f, std::max(0.0f, -d.dot(n)));
    float kr = fresnelSchlick(cos_theta, ior_from, ior_to);

    vec3 refl_dir = reflect(d, n);
    vec3 refl_color = shadeRay(Ray(hit->point, refl_dir), depth - 1);

    auto refr_dir = refract(d, n, ior_from / ior_to);
    if (!refr_dir.has_value()) {
      // Total internal reflection — all energy goes into reflection.
      return refl_color;
    }
    vec3 refr_color = shadeRay(Ray(hit->point, *refr_dir), depth - 1);
    refr_color = refr_color * hit->material.transmission;

    return refl_color * kr + refr_color * (1.0f - kr);
  }
  }
  return background;
}

// --- Photon emission for caustics ---

namespace {
// Sample a unit vector uniformly over the lower hemisphere (z <= 0).
vec3 sampleLowerHemisphere(std::mt19937 &rng, std::uniform_real_distribution<float> &dist)
{
  float u1 = dist(rng);
  float u2 = dist(rng);
  float phi = 2.0f * static_cast<float>(M_PI) * u1;
  float cos_theta = -u2;  // [-1, 0]
  float sin_theta = std::sqrt(std::max(0.0f, 1.0f - cos_theta * cos_theta));
  return vec3(std::cos(phi) * sin_theta, std::sin(phi) * sin_theta, cos_theta);
}
}  // namespace

void Shader::buildCausticsMap(CausticsMap &map, int numPhotons, int maxBounces) const
{
  const auto &lights = scene.getLights();
  if (lights.empty() || numPhotons <= 0) return;

  // For simplicity we emit from the first light and split photon budget across all lights.
  // For the assignment scene there is exactly one point light.
  unsigned int threadCount = numThreads > 0 ? numThreads : std::thread::hardware_concurrency();
  if (threadCount == 0) threadCount = 4;

  // Each emitted photon represents a fraction of the lower-hemisphere flux.
  // hemisphere solid angle = 2π. Per-photon power = 2π * intensity / N.
  const float TWO_PI = 2.0f * static_cast<float>(M_PI);

  auto emitChunk = [&](int start, int end) {
    thread_local std::mt19937 rng(std::random_device{}());
    thread_local std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    for (const auto &light : lights) {
      vec3 light_pos = light.getPosition();
      vec3 base_power = light.getIntensity() * (TWO_PI / static_cast<float>(numPhotons));

      for (int i = start; i < end; ++i) {
        vec3 dir = sampleLowerHemisphere(rng, dist);
        Ray photon(light_pos, dir);
        vec3 power = base_power;
        bool has_specular = false;

        for (int bounce = 0; bounce < maxBounces; ++bounce) {
          auto hit = scene.traceRayWithHitInfo(photon);
          if (!hit.has_value()) break;

          ShaderType type = hit->material.shaderType.value_or(defaultShaderType);

          if (type == ShaderType::DIELECTRIC) {
            vec3 d = photon.getDirection().normalized();
            vec3 n = hit->is_front_face ? hit->normal : (hit->normal * -1.0f);
            float ior_from = hit->is_front_face ? 1.0f : hit->material.ior;
            float ior_to   = hit->is_front_face ? hit->material.ior : 1.0f;
            float cos_theta = std::min(1.0f, std::max(0.0f, -d.dot(n)));
            float kr = fresnelSchlick(cos_theta, ior_from, ior_to);
            auto refr_dir = refract(d, n, ior_from / ior_to);

            // Russian-roulette split between reflection and transmission.
            if (!refr_dir.has_value() || dist(rng) < kr) {
              // Reflect.
              vec3 refl_dir = reflect(d, n);
              photon = Ray(hit->point, refl_dir);
            } else {
              // Refract; tint power by transmission.
              power = power * hit->material.transmission;
              photon = Ray(hit->point, *refr_dir);
            }
            has_specular = true;
            continue;
          }

          if (type == ShaderType::MIRROR) {
            vec3 d = photon.getDirection().normalized();
            vec3 refl_dir = reflect(d, hit->normal);
            photon = Ray(hit->point, refl_dir);
            power = power * hit->material.color;  // mirror tints
            has_specular = true;
            continue;
          }

          if (type == ShaderType::DIFFUSE || type == ShaderType::PATH_DIFFUSE) {
            // Photon terminates here. Deposit only if it was previously bent
            // through a specular surface (so direct-light hits are excluded).
            if (has_specular && hit->material.is_caustic_receiver) {
              map.deposit(hit->point, power);
            }
            break;
          }

          // SIMPLE / NORMAL / BLINN_PHONG: not part of the caustics model.
          break;
        }
      }
    }
  };

  // Distribute photons across threads.
  std::vector<std::thread> threads;
  int per = numPhotons / static_cast<int>(threadCount);
  int rem = numPhotons % static_cast<int>(threadCount);
  int start = 0;
  for (unsigned int t = 0; t < threadCount; ++t) {
    int end = start + per + (t < static_cast<unsigned int>(rem) ? 1 : 0);
    threads.emplace_back(emitChunk, start, end);
    start = end;
  }
  for (auto &th : threads) th.join();
}
