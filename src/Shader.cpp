#include "Shader.h"
#include <cmath>

// --- Free helper functions for per-object shader dispatch ---

static vec3 computeDiffuse(const Scene& scene, const HitRecord& hit, bool shadows) {
    vec3 shaded_color(0.0f, 0.0f, 0.0f);
    const auto& lights = scene.getLights();
    for (const auto& light : lights) {
        vec3 to_light = light.getPosition() - hit.point;
        float light_dist = to_light.length();
        vec3 light_dir = to_light / light_dist;

        if (shadows && scene.isOccluded(Ray(hit.point, light_dir), light_dist)) continue;

        float diffuse_factor = std::max(0.0f, hit.normal.dot(light_dir));
        shaded_color = shaded_color + (hit.material.color * light.getIntensity() * diffuse_factor);
    }
    // Ambient is always applied regardless of occlusion
    vec3 ambient(0.1f, 0.1f, 0.1f);
    shaded_color = shaded_color + (hit.material.color * ambient);
    return shaded_color;
}

static vec3 computeBlinnPhong(const Scene& scene, const Ray& ray, const HitRecord& hit, bool shadows) {
    vec3 view_dir = (ray.getDirection() * -1.0f).normalized();
    vec3 shaded_color(0.0f, 0.0f, 0.0f);
    const auto& lights = scene.getLights();
    for (const auto& light : lights) {
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
    // Ambient is always applied regardless of occlusion
    vec3 ambient(0.1f, 0.1f, 0.1f);
    shaded_color = shaded_color + (hit.material.color * ambient);
    return shaded_color;
}

// --- Base Shader: common traceScene loop ---

void Shader::traceScene(FrameBuffer& fb, int raysPerPixel) const {
    const Camera& camera = scene.getCamera();
    float w = fb.getWidth();
    float h = fb.getHeight();
    for (size_t y = 0; y < static_cast<size_t>(fb.getHeight()); ++y) {
        for (size_t x = 0; x < static_cast<size_t>(fb.getWidth()); ++x) {
            vec3 color(0.0f, 0.0f, 0.0f);

            for (int sample = 0; sample < raysPerPixel; ++sample) {
                Ray ray = (raysPerPixel > 1) ? camera.generateRayAA(x, y, w, h) : camera.generateRay(x, y, w, h);
                color = color + shadeRay(ray);
            }

            color = color * (1.0f / static_cast<float>(raysPerPixel));
            fb.setPixel(x, y, color);
        }
    }
}

// --- SimpleShader ---

vec3 SimpleShader::shadeRay(const Ray& ray) const {
    return scene.traceRay(ray, background);
}

// --- NormalShader ---

vec3 NormalShader::shadeRay(const Ray& ray) const {
    auto hit = scene.traceRayWithHitInfo(ray);
    if (!hit.has_value()) return background;

    vec3 normal = hit->normal;
    return vec3(
        (normal[0] + 1.0f) * 0.5f,
        (normal[1] + 1.0f) * 0.5f,
        (normal[2] + 1.0f) * 0.5f
    );
}

// --- DiffuseShader ---

vec3 DiffuseShader::shadeRay(const Ray& ray) const {
    return shadeRay(ray, 5);
}

vec3 DiffuseShader::shadeRay(const Ray& ray, int depth) const {
    if (depth <= 0) return background;

    auto hit = scene.traceRayWithHitInfo(ray);
    if (!hit.has_value()) return background;

    if (hit->material.type == Material::METALLIC) {
        vec3 reflected_dir = reflect(ray.getDirection().normalized(), hit->normal);
        reflected_dir = (reflected_dir + random_in_unit_sphere() * hit->material.fuzziness).normalized();
        Ray reflected_ray(hit->point, reflected_dir);
        return hit->material.color * shadeRay(reflected_ray, depth - 1);
    }

    // Dispatch based on per-object shader override, defaulting to diffuse
    switch (hit->material.shaderType.value_or(ShaderType::DIFFUSE)) {
        case ShaderType::SIMPLE:
            return hit->material.color;
        case ShaderType::NORMAL:
            return vec3(
                (hit->normal[0] + 1.0f) * 0.5f,
                (hit->normal[1] + 1.0f) * 0.5f,
                (hit->normal[2] + 1.0f) * 0.5f
            );
        case ShaderType::BLINN_PHONG:
            return computeBlinnPhong(scene, ray, *hit, shadows);
        case ShaderType::MIRROR: {
            vec3 reflected_dir = reflect(ray.getDirection().normalized(), hit->normal);
            Ray reflected_ray(hit->point, reflected_dir);
            return hit->material.color * shadeRay(reflected_ray, depth - 1);
        }
        default:
            return computeDiffuse(scene, *hit, shadows);
    }
}

// --- BlinnPhongShader ---

vec3 BlinnPhongShader::shadeRay(const Ray& ray) const {
    return shadeRay(ray, 5);
}

vec3 BlinnPhongShader::shadeRay(const Ray& ray, int depth) const {
    if (depth <= 0) return background;

    auto hit = scene.traceRayWithHitInfo(ray);
    if (!hit.has_value()) return background;

    if (hit->material.type == Material::METALLIC) {
        vec3 reflected_dir = reflect(ray.getDirection().normalized(), hit->normal);
        reflected_dir = (reflected_dir + random_in_unit_sphere() * hit->material.fuzziness).normalized();
        Ray reflected_ray(hit->point, reflected_dir);
        return hit->material.color * shadeRay(reflected_ray, depth - 1);
    }

    // Dispatch based on per-object shader override, defaulting to Blinn-Phong
    switch (hit->material.shaderType.value_or(ShaderType::BLINN_PHONG)) {
        case ShaderType::SIMPLE:
            return hit->material.color;
        case ShaderType::NORMAL:
            return vec3(
                (hit->normal[0] + 1.0f) * 0.5f,
                (hit->normal[1] + 1.0f) * 0.5f,
                (hit->normal[2] + 1.0f) * 0.5f
            );
        case ShaderType::DIFFUSE:
            return computeDiffuse(scene, *hit, shadows);
        case ShaderType::MIRROR: {
            vec3 reflected_dir = reflect(ray.getDirection().normalized(), hit->normal);
            Ray reflected_ray(hit->point, reflected_dir);
            return hit->material.color * shadeRay(reflected_ray, depth - 1);
        }
        default:
            return computeBlinnPhong(scene, ray, *hit, shadows);
    }
}

// --- MirrorShader ---

vec3 MirrorShader::shadeRay(const Ray& ray) const {
    return shadeRay(ray, maxDepth);
}

vec3 MirrorShader::shadeRay(const Ray& ray, int depth) const {
    if (depth <= 0) return background;

    auto hit = scene.traceRayWithHitInfo(ray);
    if (!hit.has_value()) return background;

    // All surfaces reflect; tint by material color
    vec3 reflected_dir = reflect(ray.getDirection().normalized(), hit->normal);
    // Respect fuzziness for metallic materials
    if (hit->material.type == Material::METALLIC && hit->material.fuzziness > 0.0f) {
        reflected_dir = (reflected_dir + random_in_unit_sphere() * hit->material.fuzziness).normalized();
    }
    Ray reflected_ray(hit->point, reflected_dir);
    return hit->material.color * shadeRay(reflected_ray, depth - 1);
}
