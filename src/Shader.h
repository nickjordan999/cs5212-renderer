#ifndef SHADER_H
#define SHADER_H

#include "Camera.h"
#include "Scene.h"
#include "FrameBuffer.h"
#include "Material.h"
#include "vec.h"

// Abstract base class for shaders
class Shader {
public:
    virtual ~Shader() = default;

    // Main method to trace the scene and fill the framebuffer
    // raysPerPixel: number of rays per pixel for anti-aliasing
    virtual void traceScene(FrameBuffer& fb, int raysPerPixel) const = 0;

protected:
    Shader(const Scene& scene, const vec3& background)
        : scene(scene), background(background) {}

    const Scene& scene;
    vec3 background;
};

// Standard render shader - renders with material colors
class SimpleShader : public Shader {
public:
    SimpleShader(const Scene& scene)
        : Shader(scene, vec3(0.5f, 0.7f, 1.0f)) {}  // sky blue background

    void traceScene(FrameBuffer& fb, int raysPerPixel) const override;
};

// Normal shader - visualizes surface normals
class NormalShader : public Shader {
public:
    NormalShader(const Scene& scene)
        : Shader(scene, vec3(0.2f, 0.2f, 0.2f)) {}  // dark gray background

    void traceScene(FrameBuffer& fb, int raysPerPixel) const override;
};

// Diffuse shader - renders with diffuse shading from lights
class DiffuseShader : public Shader {
public:
    DiffuseShader(const Scene& scene)
        : Shader(scene, vec3(0.2f, 0.2f, 0.2f)) {}  // dark gray background

    void traceScene(FrameBuffer& fb, int raysPerPixel) const override;

private:
    vec3 shadeRay(const Ray& ray, int depth) const;
};

#endif // SHADER_H
