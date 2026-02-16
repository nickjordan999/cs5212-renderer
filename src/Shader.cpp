#include "Shader.h"
#include <cmath>

void SimpleShader::traceScene(FrameBuffer& fb, int raysPerPixel) const {
    const Camera& camera = scene.getCamera();
    float w = fb.getWidth();
    float h = fb.getHeight();
    // Raytrace the scene
    for (size_t y = 0; y < static_cast<size_t>(fb.getHeight()); ++y) {
        for (size_t x = 0; x < static_cast<size_t>(fb.getWidth()); ++x) {
            vec3 color(0.0f, 0.0f, 0.0f);

            // Sample multiple rays per pixel for anti-aliasing
            for (int sample = 0; sample < raysPerPixel; ++sample) {
                Ray ray = (raysPerPixel > 1) ? camera.generateRayAA(x, y, w, h) : camera.generateRay(x, y, w, h);
                color = color + scene.traceRay(ray, background);
            }
            
            // Average the samples
            color = color * (1.0f / static_cast<float>(raysPerPixel));
            fb.setPixel(x, y, color);
        }
    }
}

void NormalShader::traceScene(FrameBuffer& fb, int raysPerPixel) const {
    const Camera& camera = scene.getCamera();
    float w = fb.getWidth();
    float h = fb.getHeight();
    // Raytrace the scene with normal visualization
    for (size_t y = 0; y < static_cast<size_t>(fb.getHeight()); ++y) {
        for (size_t x = 0; x < static_cast<size_t>(fb.getWidth()); ++x) {
            vec3 color(0.0f, 0.0f, 0.0f);

            // Sample multiple rays per pixel for anti-aliasing
            for (int sample = 0; sample < raysPerPixel; ++sample) {
                Ray ray = (raysPerPixel > 1) ? camera.generateRayAA(x, y, w, h) : camera.generateRay(x, y, w, h);
                auto hit = scene.traceRayWithHitInfo(ray);

                vec3 sample_color;
                if (hit.has_value()) {
                    // Map normal direction to color
                    // Normal components range from -1 to 1, we map to 0 to 1
                    vec3 normal = hit->normal;
                    sample_color = vec3(
                        (normal[0] + 1.0f) * 0.5f,
                        (normal[1] + 1.0f) * 0.5f,
                        (normal[2] + 1.0f) * 0.5f
                    );
                } else {
                    sample_color = background;
                }
                
                color = color + sample_color;
            }
            
            // Average the samples
            color = color * (1.0f / static_cast<float>(raysPerPixel));
            fb.setPixel(x, y, color);
        }
    }
}

vec3 DiffuseShader::shadeRay(const Ray& ray, int depth) const {
    if (depth <= 0) return background;

    auto hit = scene.traceRayWithHitInfo(ray);
    if (!hit.has_value()) return background;

    if (hit->material.type == Material::METALLIC) {
        // Compute reflected ray
        vec3 reflected_dir = reflect(ray.getDirection().normalized(), hit->normal);
        reflected_dir = (reflected_dir + random_in_unit_sphere() * hit->material.fuzziness).normalized();
        Ray reflected_ray(hit->point, reflected_dir);
        // Tint recursive result by metal color
        return hit->material.color * shadeRay(reflected_ray, depth - 1);
    }

    // Diffuse shading: sum contributions from all lights
    vec3 shaded_color(0.0f, 0.0f, 0.0f);

    const auto& lights = scene.getLights();
    for (const auto& light : lights) {
        vec3 light_dir = (light.getPosition() - hit->point).normalized();
        float diffuse_factor = std::max(0.0f, hit->normal.dot(light_dir));
        shaded_color = shaded_color + (hit->material.color * light.getIntensity() * diffuse_factor);
    }

    // Add ambient light
    vec3 ambient(0.1f, 0.1f, 0.1f);
    shaded_color = shaded_color + (hit->material.color * ambient);

    return shaded_color;
}

void DiffuseShader::traceScene(FrameBuffer& fb, int raysPerPixel) const {
    const Camera& camera = scene.getCamera();
    float w = fb.getWidth();
    float h = fb.getHeight();
    // Raytrace the scene with diffuse shading
    for (size_t y = 0; y < static_cast<size_t>(fb.getHeight()); ++y) {
        for (size_t x = 0; x < static_cast<size_t>(fb.getWidth()); ++x) {
            vec3 color(0.0f, 0.0f, 0.0f);

            // Sample multiple rays per pixel for anti-aliasing
            for (int sample = 0; sample < raysPerPixel; ++sample) {
                Ray ray = (raysPerPixel > 1) ? camera.generateRayAA(x, y, w, h) : camera.generateRay(x, y, w, h);
                color = color + shadeRay(ray, 5);
            }

            // Average the samples
            color = color * (1.0f / static_cast<float>(raysPerPixel));
            fb.setPixel(x, y, color);
        }
    }
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

    // Blinn-Phong shading
    vec3 view_dir = (ray.getDirection() * -1.0f).normalized();
    vec3 shaded_color(0.0f, 0.0f, 0.0f);

    const auto& lights = scene.getLights();
    for (const auto& light : lights) {
        vec3 light_dir = (light.getPosition() - hit->point).normalized();

        // Diffuse
        float diff = std::max(0.0f, hit->normal.dot(light_dir));

        // Specular (Blinn-Phong halfway vector)
        vec3 halfway = (light_dir + view_dir).normalized();
        float spec = std::pow(std::max(0.0f, hit->normal.dot(halfway)), 32.0f);

        shaded_color = shaded_color
            + hit->material.color * light.getIntensity() * diff
            + light.getIntensity() * spec * 0.5f;
    }

    // Ambient
    vec3 ambient(0.1f, 0.1f, 0.1f);
    shaded_color = shaded_color + (hit->material.color * ambient);

    return shaded_color;
}

void BlinnPhongShader::traceScene(FrameBuffer& fb, int raysPerPixel) const {
    const Camera& camera = scene.getCamera();
    float w = fb.getWidth();
    float h = fb.getHeight();
    for (size_t y = 0; y < static_cast<size_t>(fb.getHeight()); ++y) {
        for (size_t x = 0; x < static_cast<size_t>(fb.getWidth()); ++x) {
            vec3 color(0.0f, 0.0f, 0.0f);

            for (int sample = 0; sample < raysPerPixel; ++sample) {
                Ray ray = (raysPerPixel > 1) ? camera.generateRayAA(x, y, w, h) : camera.generateRay(x, y, w, h);
                color = color + shadeRay(ray, 5);
            }

            color = color * (1.0f / static_cast<float>(raysPerPixel));
            fb.setPixel(x, y, color);
        }
    }
}
