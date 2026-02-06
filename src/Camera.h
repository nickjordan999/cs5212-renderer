#ifndef CAMERA_H
#define CAMERA_H

#include "vec.h"
#include "Ray.h"
#include <cmath>
#include <random>

// Abstract base class for cameras
class Camera {
public:
    virtual ~Camera() = default;

    // Generate a ray that passes through pixel (i, j)
    // i: horizontal pixel coordinate (0 to width)
    // j: vertical pixel coordinate (0 to height)
    virtual Ray generateRay(float i, float j) const = 0;

    // Generate a ray with anti-aliasing jitter for pixel (i, j)
    // i: horizontal pixel coordinate (0 to width)
    // j: vertical pixel coordinate (0 to height)
    // This generates a ray within a slightly offset region of the pixel for anti-aliasing
    virtual Ray generateRayAA(float i, float j) const = 0;

    virtual float getWidth() const = 0;
    virtual float getHeight() const = 0;

protected:
    // Static random number generator for anti-aliasing
    static thread_local std::mt19937 rng;
    static thread_local std::uniform_real_distribution<float> distribution;
    
    static float randomFloat() {
        return distribution(rng);
    }
};

// Basic Perspective camera following implementation from Ray Tracing in One Weekend

class PerspectiveBasicCamera : public Camera {
public:
    // Constructor
    // position: camera position (eye point)
    // direction: viewing direction (normalized)
    // focal_length: focal length (distance to image plane)
    // width: image width in pixels
    // height: image height in pixels
    PerspectiveBasicCamera(const vec3& position, const vec3& direction, float focal_length,
                           float width, float height)
        : position(position), direction(direction), focal_length(focal_length),
          width(width), height(height) {
            setupCamera();
    }

    Ray generateRay(float i, float j) const override {

        // Normalize pixel coordinates to [-1, 1] range (with aspect ratio)
        float aspect = width / height;
        float x = (2.0f * i / width - 1.0f) * aspect;
        float y = 1.0f - 2.0f * j / height;

        // Ray direction in camera space
        vec3 ray_direction = (focal_length * direction + u * x + v * y); // we are leaving this unnormalized

        return Ray(position, ray_direction);
    }

    Ray generateRayAA(float i, float j) const override {
        // Add random jitter within the pixel bounds [0, 1)
        float jitter_x = randomFloat();
        float jitter_y = randomFloat();

        // Normalize pixel coordinates with jitter to [-1, 1] range (with aspect ratio)
        float aspect = width / height;
        float x = (2.0f * (i + jitter_x) / width - 1.0f) * aspect;
        float y = 1.0f - 2.0f * (j + jitter_y) / height;

        // Ray direction in camera space
        vec3 ray_direction = (focal_length * direction + u * x + v * y); // we are leaving this unnormalized

        return Ray(position, ray_direction);
    }

    float getWidth() const override { return width; }
    float getHeight() const override { return height; }

private:
    void setupCamera() {
        // Assume up vector is (0, 1, 0)
        vec3 up(0.0f, 1.0f, 0.0f);
        u = direction.cross(up).normalized();
        v = direction.cross(u).normalized();
    }
    vec3 position;
    vec3 direction;
    vec3 u;
    vec3 v;

    float focal_length;
    float width;
    float height;

};  

// Perspective camera implementation
class PerspectiveFOVCamera : public Camera {
public:
    // Constructor
    // position: camera position (eye point)
    // target: point the camera is looking at
    // up: up vector (typically (0, 1, 0))
    // fov: field of view in degrees (vertical)
    // width: image width in pixels
    // height: image height in pixels
    PerspectiveFOVCamera(const vec3& position, const vec3& target, const vec3& up,
                      float fov, float width, float height)
        : position(position), width(width), height(height) {
        setupCamera(position, target, up, fov);
    }

    Ray generateRay(float i, float j) const override {
        // Normalize pixel coordinates to [-1, 1] range (with aspect ratio)
        float aspect = width / height;
        float x = (2.0f * i / width - 1.0f) * aspect;
        float y = 1.0f - 2.0f * j / height;

        // Ray direction in camera space
        vec3 direction = (forward + right * x + up_normalized * y).normalized();

        return Ray(position, direction);
    }

    Ray generateRayAA(float i, float j) const override {
        // Add random jitter within the pixel bounds [0, 1)
        float jitter_x = randomFloat();
        float jitter_y = randomFloat();

        // Normalize pixel coordinates with jitter to [-1, 1] range (with aspect ratio)
        float aspect = width / height;
        float x = (2.0f * (i + jitter_x) / width - 1.0f) * aspect;
        float y = 1.0f - 2.0f * (j + jitter_y) / height;

        // Ray direction in camera space
        vec3 direction = (forward + right * x + up_normalized * y).normalized();

        return Ray(position, direction);
    }

    float getWidth() const override { return width; }
    float getHeight() const override { return height; }

private:
    void setupCamera(const vec3& pos, const vec3& target, const vec3& up, float fov) {
        // Calculate camera basis vectors
        forward = (target - pos).normalized();
        up_normalized = up.normalized();
        right = forward.cross(up_normalized).normalized();
        
        // Recalculate up to ensure orthogonal basis
        up_normalized = right.cross(forward).normalized();

        // Calculate the height of the viewport at distance 1 from camera
        float fov_rad = fov * M_PI / 180.0f;
        viewport_height = 2.0f * std::tan(fov_rad / 2.0f);
    }

    vec3 position;
    vec3 forward;
    vec3 right;
    vec3 up_normalized;
    float width;
    float height;
    float viewport_height;
};

// Orthographic camera implementation
class OrthographicCamera : public Camera {
public:
    // Constructor
    // position: camera position
    // target: point the camera is looking at
    // up: up vector (typically (0, 1, 0))
    // height: height of the view frustum
    // width: image width in pixels
    // height_pixels: image height in pixels
    OrthographicCamera(const vec3& position, const vec3& target, const vec3& up,
                       float height, float width_pixels, float height_pixels)
        : position(position), width(width_pixels), height(height_pixels),
          view_height(height) {
        setupCamera(position, target, up);
    }

    Ray generateRay(float i, float j) const override {
        // Normalize pixel coordinates to [-0.5, 0.5] range (scaled by view height)
        float aspect = width / height;
        float x = (i / width - 0.5f) * view_height * aspect;
        float y = (0.5f - j / height) * view_height;

        // Ray origin on the image plane
        vec3 ray_origin = position + right * x + up_normalized * y;

        // Ray direction is always forward for orthographic
        return Ray(ray_origin, forward);
    }

    Ray generateRayAA(float i, float j) const override {
        // Add random jitter within the pixel bounds [0, 1)
        float jitter_x = randomFloat();
        float jitter_y = randomFloat();

        // Normalize pixel coordinates with jitter to [-0.5, 0.5] range (scaled by view height)
        float aspect = width / height;
        float x = ((i + jitter_x) / width - 0.5f) * view_height * aspect;
        float y = (0.5f - (j + jitter_y) / height) * view_height;

        // Ray origin on the image plane
        vec3 ray_origin = position + right * x + up_normalized * y;

        // Ray direction is always forward for orthographic
        return Ray(ray_origin, forward);
    }

    float getWidth() const override { return width; }
    float getHeight() const override { return height; }

private:
    void setupCamera(const vec3& pos, const vec3& target, const vec3& up) {
        // Calculate camera basis vectors
        forward = (target - pos).normalized();
        up_normalized = up.normalized();
        right = forward.cross(up_normalized).normalized();
        
        // Recalculate up to ensure orthogonal basis
        up_normalized = right.cross(forward).normalized();
    }

    vec3 position;
    vec3 forward;
    vec3 right;
    vec3 up_normalized;
    float width;
    float height;
    float view_height;
};

#endif // CAMERA_H
