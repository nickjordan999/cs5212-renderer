#include "FrameBuffer.h"
#include "vec.h"
#include "Ray.h"
#include "Camera.h"
#include "Scene.h"
#include "Sphere.h"
#include <iostream>
#include <fstream>
#include <cstdio>
#include <cmath>
#include <unistd.h>
#include <string>

void printUsage(const char* programName) {
    std::cerr << "Usage:" << std::endl;
    std::cerr << "  " << programName << " [mode] <width> <height> <focal_length>" << std::endl;
    std::cerr << std::endl;
    std::cerr << "  Modes:" << std::endl;
    std::cerr << "    (default/render)     - Renders scene with material colors" << std::endl;
    std::cerr << "    normalshader         - Renders scene with normal visualization" << std::endl;
    std::cerr << std::endl;
    std::cerr << "  Examples:" << std::endl;
    std::cerr << "    " << programName << " 800 600 0.25" << std::endl;
    std::cerr << "    " << programName << " normalshader 800 600 0.25" << std::endl;
}

void render(FrameBuffer& fb, int width, int height, float focal_length) {
    // Create camera
    PerspectiveBasicCamera camera(
        vec3(0.0f, 0.0f, 3.0f), vec3(0.0f, 0.0f, -1.0f), focal_length, 
        static_cast<float>(width), static_cast<float>(height)
    );

    // Create scene with some spheres
    Scene scene;

    // Large central sphere
    scene.addSphere(Sphere(vec3(0.0f, 0.0f, -5.0f), 1.0f, vec3(1.0f, 0.0f, 0.0f)));     // Red sphere

    // Medium spheres to the sides
    scene.addSphere(Sphere(vec3(-2.5f, 0.0f, -6.0f), 0.667f, vec3(0.0f, 1.0f, 0.0f)));    // Green sphere
    scene.addSphere(Sphere(vec3(2.5f, 0.0f, -6.0f), 0.667f, vec3(0.0f, 0.0f, 1.0f)));     // Blue sphere

    // Small spheres on the top and bottom
    scene.addSphere(Sphere(vec3(0.0f, -2.0f, -6.0f), 0.333f, vec3(1.0f, 1.0f, 1.0f)));    // Green sphere
    scene.addSphere(Sphere(vec3(0.0f, 2.0f, -6.0f), 0.333f, vec3(0.0f, 0.0f, 0.0f)));     // Blue sphere

    // Background color (sky blue)
    vec3 background(0.5f, 0.7f, 1.0f);

    // Raytrace the scene
    for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; ++x) {
            Ray ray = camera.generateRay(x, y);
            vec3 color = scene.traceRay(ray, background);
            fb.setPixel(x, y, color);
        }
    }
}

void normalShader(FrameBuffer& fb, int width, int height, float focal_length) {
    // Create camera
    PerspectiveBasicCamera camera(
        vec3(0.0f, 0.0f, 3.0f), vec3(0.0f, 0.0f, -1.0f), focal_length, 
        static_cast<float>(width), static_cast<float>(height)
    );

    // Create scene with some spheres
    Scene scene;

    // Large central sphere
    scene.addSphere(Sphere(vec3(0.0f, 0.0f, -5.0f), 1.0f, vec3(1.0f, 0.0f, 0.0f)));     // Red sphere

    // Medium spheres to the sides
    scene.addSphere(Sphere(vec3(-2.5f, 0.0f, -6.0f), 0.667f, vec3(0.0f, 1.0f, 0.0f)));    // Green sphere
    scene.addSphere(Sphere(vec3(2.5f, 0.0f, -6.0f), 0.667f, vec3(0.0f, 0.0f, 1.0f)));     // Blue sphere

    // Small spheres on the top and bottom
    scene.addSphere(Sphere(vec3(0.0f, -2.0f, -6.0f), 0.333f, vec3(1.0f, 1.0f, 1.0f)));    // Green sphere
    scene.addSphere(Sphere(vec3(0.0f, 2.0f, -6.0f), 0.333f, vec3(0.0f, 0.0f, 0.0f)));     // Blue sphere

    // Background color
    vec3 background(0.2f, 0.2f, 0.2f);

    // Raytrace the scene with normal visualization
    for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; ++x) {
            Ray ray = camera.generateRay(x, y);
            auto hit = scene.traceRayWithHitInfo(ray);

            vec3 color;
            if (hit.has_value()) {
                // Map normal direction to color
                // Normal components range from -1 to 1, we map to 0 to 1
                vec3 normal = hit->normal;
                color = vec3(
                    (normal[0] + 1.0f) * 0.5f,
                    (normal[1] + 1.0f) * 0.5f,
                    (normal[2] + 1.0f) * 0.5f
                );
            } else {
                color = background;
            }

            fb.setPixel(x, y, color);
        }
    }
}

int main(int argc, char* argv[]) {
    std::string mode = "testcamera";
    int width = 0;
    int height = 0;
    float focal_length = 0.25f;

    // Parse arguments
    if (argc == 3) {
        // Format: raytracer <width> <height>
        // Uses default mode (testcamera) and focal_length (0.25)
        try {
            width = std::stoi(argv[1]);
            height = std::stoi(argv[2]);
        } catch (...) {
            printUsage(argv[0]);
            return 1;
        }
    } else if (argc == 4) {
        // Format: raytracer <width> <height> <focal_length>
        // or: raytracer <mode> <width> <height> (less common, but try this first)
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
        mode = argv[1];
        try {
            width = std::stoi(argv[2]);
            height = std::stoi(argv[3]);
            focal_length = std::stof(argv[4]);
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

        FrameBuffer fb(width, height);

        if (mode == "render") {
            render(fb, width, height, focal_length);
        } else if (mode == "normalshader") {
            normalShader(fb, width, height, focal_length);
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
