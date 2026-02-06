#include "Shader.h"
#include <cmath>

void SimpleShader::traceScene(FrameBuffer& fb, int raysPerPixel) const {
    const Camera& camera = scene.getCamera();
    // Raytrace the scene
    for (size_t y = 0; y < static_cast<size_t>(fb.getHeight()); ++y) {
        for (size_t x = 0; x < static_cast<size_t>(fb.getWidth()); ++x) {
            vec3 color(0.0f, 0.0f, 0.0f);
            
            // Sample multiple rays per pixel for anti-aliasing
            for (int sample = 0; sample < raysPerPixel; ++sample) {
                Ray ray = (raysPerPixel > 1) ? camera.generateRayAA(x, y) : camera.generateRay(x, y);
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
    // Raytrace the scene with normal visualization
    for (size_t y = 0; y < static_cast<size_t>(fb.getHeight()); ++y) {
        for (size_t x = 0; x < static_cast<size_t>(fb.getWidth()); ++x) {
            vec3 color(0.0f, 0.0f, 0.0f);
            
            // Sample multiple rays per pixel for anti-aliasing
            for (int sample = 0; sample < raysPerPixel; ++sample) {
                Ray ray = (raysPerPixel > 1) ? camera.generateRayAA(x, y) : camera.generateRay(x, y);
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

void DiffuseShader::traceScene(FrameBuffer& fb, int raysPerPixel) const {
    const Camera& camera = scene.getCamera();
    // Raytrace the scene with diffuse shading
    for (size_t y = 0; y < static_cast<size_t>(fb.getHeight()); ++y) {
        for (size_t x = 0; x < static_cast<size_t>(fb.getWidth()); ++x) {
            vec3 color(0.0f, 0.0f, 0.0f);
            
            // Sample multiple rays per pixel for anti-aliasing
            for (int sample = 0; sample < raysPerPixel; ++sample) {
                Ray ray = (raysPerPixel > 1) ? camera.generateRayAA(x, y) : camera.generateRay(x, y);
                auto hit = scene.traceRayWithHitInfo(ray);

                vec3 sample_color;
                if (hit.has_value()) {
                    // Diffuse shading: sum contributions from all lights
                    vec3 shaded_color(0.0f, 0.0f, 0.0f);
                    
                    const auto& lights = scene.getLights();
                    for (const auto& light : lights) {
                        // Calculate light direction (normalized)
                        vec3 light_dir = (light.getPosition() - hit->point).normalized();
                        
                        // Calculate diffuse factor using Lambert's cosine law
                        // Only positive dot products contribute (front-facing surfaces)
                        float diffuse_factor = std::max(0.0f, hit->normal.dot(light_dir));
                        
                        // Accumulate light contribution: material_color * light_intensity * diffuse_factor
                        shaded_color = shaded_color + (hit->material * light.getIntensity() * diffuse_factor);
                    }
                    
                    // Add ambient light
                    vec3 ambient(0.1f, 0.1f, 0.1f);
                    shaded_color = shaded_color + (hit->material * ambient);
                    
                    sample_color = shaded_color;
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
