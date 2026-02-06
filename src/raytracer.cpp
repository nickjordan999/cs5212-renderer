#include "FrameBuffer.h"
#include "vec.h"
#include "Ray.h"
#include "Camera.h"
#include "Scene.h"
#include "ScenePresets.h"
#include "Sphere.h"
#include "Light.h"
#include <iostream>
#include <fstream>
#include <cstdio>
#include <cmath>
#include <unistd.h>
#include <string>

void printUsage(const char* programName) {
    std::cerr << "Usage:" << std::endl;
    std::cerr << "  " << programName << " [mode] <width> <height> <focal_length> [scene] [raysPerPixel]" << std::endl;
    std::cerr << std::endl;
    std::cerr << "  Modes:" << std::endl;
    std::cerr << "    (default/render)     - Renders scene with material colors" << std::endl;
    std::cerr << "    normalshader         - Renders scene with normal visualization" << std::endl;
    std::cerr << "    diffuse              - Renders scene with diffuse shading" << std::endl;
    std::cerr << std::endl;
    std::cerr << "  Scenes:" << std::endl;
    std::cerr << "    test                 - Standard test scene with colored spheres (default)" << std::endl;
    std::cerr << "    single_sphere        - Single purple sphere" << std::endl;
    std::cerr << "    grid                 - 3x3 grid of spheres" << std::endl;
    std::cerr << std::endl;
    std::cerr << "  Anti-aliasing:" << std::endl;
    std::cerr << "    raysPerPixel         - Number of rays per pixel for anti-aliasing (default: 1, no AA)" << std::endl;
    std::cerr << std::endl;
    std::cerr << "  Examples:" << std::endl;
    std::cerr << "    " << programName << " 800 600 0.25" << std::endl;
    std::cerr << "    " << programName << " normalshader 800 600 0.25" << std::endl;
    std::cerr << "    " << programName << " diffuse 800 600 0.25 grid" << std::endl;
    std::cerr << "    " << programName << " 800 600 0.25 test 4" << std::endl;
}

// Load a scene by preset name
Scene loadScene(const std::string& preset_name) {
    if (preset_name == "test") {
        return ScenePresets::createTestScene();
    } else if (preset_name == "lit_test") {
        return ScenePresets::createLitTestScene();
    } else if (preset_name == "single_sphere") {
        return ScenePresets::createSingleSphereScene();
    } else if (preset_name == "grid") {
        return ScenePresets::createGridScene();
    } else {
        std::cerr << "Unknown scene preset: " << preset_name << std::endl;
        throw std::invalid_argument("Invalid scene preset");
    }
}

void render(FrameBuffer& fb, int width, int height, float focal_length, const std::string& scene_preset, int raysPerPixel) {
    // Create camera
    PerspectiveBasicCamera camera(
        vec3(0.0f, 0.0f, 3.0f), vec3(0.0f, 0.0f, -1.0f), focal_length, 
        static_cast<float>(width), static_cast<float>(height)
    );

    // Load scene preset
    Scene scene = loadScene(scene_preset);

    // Background color (sky blue)
    vec3 background(0.5f, 0.7f, 1.0f);

    // Raytrace the scene
    for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; ++x) {
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

void normalShader(FrameBuffer& fb, int width, int height, float focal_length, const std::string& scene_preset, int raysPerPixel) {
    // Create camera
    PerspectiveBasicCamera camera(
        vec3(0.0f, 0.0f, 3.0f), vec3(0.0f, 0.0f, -1.0f), focal_length, 
        static_cast<float>(width), static_cast<float>(height)
    );

    // Load scene preset
    Scene scene = loadScene(scene_preset);

    // Background color
    vec3 background(0.2f, 0.2f, 0.2f);

    // Raytrace the scene with normal visualization
    for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; ++x) {
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

void diffuseShader(FrameBuffer& fb, int width, int height, float focal_length, const std::string& scene_preset, int raysPerPixel) {
    // Create camera
    PerspectiveBasicCamera camera(
        vec3(0.0f, 0.0f, 3.0f), vec3(0.0f, 0.0f, -1.0f), focal_length, 
        static_cast<float>(width), static_cast<float>(height)
    );

    // Load scene preset (for diffuse rendering, use lit version if available)
    Scene scene;
    if (scene_preset == "test") {
        scene = ScenePresets::createLitTestScene();
    } else {
        scene = loadScene(scene_preset);
    }

    // Background color
    vec3 background(0.2f, 0.2f, 0.2f);

    // Raytrace the scene with diffuse shading
    for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; ++x) {
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

int main(int argc, char* argv[]) {
    std::string mode = "render";
    int width = 0;
    int height = 0;
    float focal_length = 0.25f;
    std::string scene_preset = "test";
    int raysPerPixel = 1;

    // Parse arguments
    if (argc == 3) {
        // Format: raytracer <width> <height>
        // Uses default mode (render), focal_length (0.25), scene (test), and raysPerPixel (1)
        try {
            width = std::stoi(argv[1]);
            height = std::stoi(argv[2]);
        } catch (...) {
            printUsage(argv[0]);
            return 1;
        }
    } else if (argc == 4) {
        // Format: raytracer <width> <height> <focal_length>
        try {
            width = std::stoi(argv[1]);
            height = std::stoi(argv[2]);
            focal_length = std::stof(argv[3]);
        } catch (...) {
            printUsage(argv[0]);
            return 1;
        }
    } else if (argc == 5) {
        // Format: raytracer <mode> <width> <height> <focal_length>
        // or: raytracer <width> <height> <focal_length> <scene>
        mode = argv[1];
        try {
            width = std::stoi(argv[2]);
            height = std::stoi(argv[3]);
            focal_length = std::stof(argv[4]);
        } catch (...) {
            // Try alternate format: width height focal_length scene
            try {
                width = std::stoi(argv[1]);
                height = std::stoi(argv[2]);
                focal_length = std::stof(argv[3]);
                scene_preset = argv[4];
                mode = "render";
            } catch (...) {
                printUsage(argv[0]);
                return 1;
            }
        }
    } else if (argc == 6) {
        // Format: raytracer <mode> <width> <height> <focal_length> <scene>
        // or: raytracer <width> <height> <focal_length> <scene> <raysPerPixel>
        mode = argv[1];
        try {
            width = std::stoi(argv[2]);
            height = std::stoi(argv[3]);
            focal_length = std::stof(argv[4]);
            scene_preset = argv[5];
        } catch (...) {
            // Try alternate format: width height focal_length scene raysPerPixel
            try {
                width = std::stoi(argv[1]);
                height = std::stoi(argv[2]);
                focal_length = std::stof(argv[3]);
                scene_preset = argv[4];
                raysPerPixel = std::stoi(argv[5]);
                mode = "render";
            } catch (...) {
                printUsage(argv[0]);
                return 1;
            }
        }
    } else if (argc == 7) {
        // Format: raytracer <mode> <width> <height> <focal_length> <scene> <raysPerPixel>
        mode = argv[1];
        try {
            width = std::stoi(argv[2]);
            height = std::stoi(argv[3]);
            focal_length = std::stof(argv[4]);
            scene_preset = argv[5];
            raysPerPixel = std::stoi(argv[6]);
        } catch (...) {
            printUsage(argv[0]);
            return 1;
        }
    } else {
        printUsage(argv[0]);
        return 1;
    }

    try {
        if (width <= 0 || height <= 0) {
            std::cerr << "Error: width and height must be positive integers" << std::endl;
            return 1;
        }

        if (raysPerPixel <= 0) {
            std::cerr << "Error: raysPerPixel must be a positive integer" << std::endl;
            return 1;
        }

        FrameBuffer fb(width, height);

        if (mode == "render") {
            render(fb, width, height, focal_length, scene_preset, raysPerPixel);
        } else if (mode == "normalshader") {
            normalShader(fb, width, height, focal_length, scene_preset, raysPerPixel);
        } else if (mode == "diffuse") {
            diffuseShader(fb, width, height, focal_length, scene_preset, raysPerPixel);
        } else {
            std::cerr << "Unknown mode: " << mode << std::endl;
            printUsage(argv[0]);
            return 1;
        }

        // Write to stdout
        char tmpfile[] = "/tmp/raytracer_XXXXXX";
        int tmpfd = mkstemp(tmpfile);
        if (tmpfd < 0) {
            std::cerr << "Error creating temporary file" << std::endl;
            return 1;
        }
        close(tmpfd);

        try {
            fb.writeToPng(tmpfile);

            std::ifstream infile(tmpfile, std::ios::binary);
            if (!infile) {
                std::cerr << "Error reading temporary PNG file" << std::endl;
                std::remove(tmpfile);
                return 1;
            }

            std::cout << infile.rdbuf();
            infile.close();

            std::remove(tmpfile);
        } catch (const std::exception& e) {
            std::remove(tmpfile);
            throw;
        }

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
