#ifndef CAMERA_H
#define CAMERA_H

#include "vec.h"
#include "Ray.h"
#include <cmath>
#include <random>

// Abstract base class for cameras
class Camera
{
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

  static float randomFloat()
  {
    return distribution(rng);
  }
};

// Basic Perspective camera following implementation from Ray Tracing in One Weekend

class PerspectiveBasicCamera : public Camera
{
public:
  // Constructor
  // position: camera position (eye point)
  // direction: viewing direction (normalized)
  // focal_length: focal length (distance to image plane)
  // width: image width in pixels
  // height: image height in pixels
  PerspectiveBasicCamera(const vec3 &position, const vec3 &direction, float focal_length, float width, float height)
    : position(position), direction(direction), focal_length(focal_length),
      width(width), height(height)
  {
    setupCamera();
  }

  Ray generateRay(float i, float j) const override
  {

    // Normalize pixel coordinates to [-1, 1] range (with aspect ratio)
    float aspect = width / height;
    float x = (2.0f * i / width - 1.0f) * aspect;
    float y = 1.0f - 2.0f * j / height;

    // Ray direction in camera space
    vec3 ray_direction = (focal_length * direction + u * x + v * y);// we are leaving this unnormalized

    return Ray(position, ray_direction);
  }

  Ray generateRayAA(float i, float j) const override
  {
    // Add random jitter within the pixel bounds [0, 1)
    float jitter_x = randomFloat();
    float jitter_y = randomFloat();

    // Normalize pixel coordinates with jitter to [-1, 1] range (with aspect ratio)
    float aspect = width / height;
    float x = (2.0f * (i + jitter_x) / width - 1.0f) * aspect;
    float y = 1.0f - 2.0f * (j + jitter_y) / height;

    // Ray direction in camera space
    vec3 ray_direction = (focal_length * direction + u * x + v * y);// we are leaving this unnormalized

    return Ray(position, ray_direction);
  }

  float getWidth() const override { return width; }
  float getHeight() const override { return height; }

private:
  void setupCamera()
  {
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
#endif// CAMERA_H
