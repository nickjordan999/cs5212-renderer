#ifndef SHADER_H
#define SHADER_H

#include "Camera.h"
#include "Scene.h"
#include "FrameBuffer.h"
#include "Material.h"
#include "vec.h"

// Abstract base class for shaders
class Shader
{
public:
  virtual ~Shader() = default;

  // Trace the scene and fill the framebuffer.
  // raysPerPixel: number of rays per pixel for anti-aliasing
  void traceScene(FrameBuffer &fb, int raysPerPixel) const;

protected:
  Shader(const Scene &scene, const vec3 &defaultBackground, bool shadows = true)
    : scene(scene), background(scene.getBackgroundColor().value_or(defaultBackground)), shadows(shadows) {}

  // Shade a single ray — each subclass implements its shading algorithm
  virtual vec3 shadeRay(const Ray &ray) const = 0;

  const Scene &scene;
  vec3 background;
  bool shadows;
};

// Standard render shader - renders with material colors
class SimpleShader : public Shader
{
public:
  SimpleShader(const Scene &scene)
    : Shader(scene, vec3(0.5f, 0.7f, 1.0f)) {}// sky blue background

protected:
  vec3 shadeRay(const Ray &ray) const override;
};

// Normal shader - visualizes surface normals
class NormalShader : public Shader
{
public:
  NormalShader(const Scene &scene)
    : Shader(scene, vec3(0.2f, 0.2f, 0.2f)) {}// dark gray background

protected:
  vec3 shadeRay(const Ray &ray) const override;
};

// Diffuse shader - renders with diffuse shading from lights
class DiffuseShader : public Shader
{
public:
  DiffuseShader(const Scene &scene, bool shadows = true)
    : Shader(scene, vec3(0.0f, 0.0f, 0.0f), shadows) {}// black background

protected:
  vec3 shadeRay(const Ray &ray) const override;

private:
  vec3 shadeRay(const Ray &ray, int depth) const;
};

// Blinn-Phong shader - ambient + diffuse + specular (halfway vector)
class BlinnPhongShader : public Shader
{
public:
  BlinnPhongShader(const Scene &scene, bool shadows = true)
    : Shader(scene, vec3(0.3f, 0.3f, 0.3f), shadows) {}

protected:
  vec3 shadeRay(const Ray &ray) const override;

private:
  vec3 shadeRay(const Ray &ray, int depth) const;
};

// Mirror shader — every surface acts as a reflective mirror.
// Material color tints the reflection; fuzziness is respected for METALLIC materials.
// maxDepth controls the maximum number of ray bounces.
class MirrorShader : public Shader
{
public:
  MirrorShader(const Scene &scene, int maxDepth = 5)
    : Shader(scene, vec3(0.5f, 0.7f, 1.0f)), maxDepth(maxDepth) {}// sky blue background

protected:
  vec3 shadeRay(const Ray &ray) const override;

private:
  int maxDepth;
  vec3 shadeRay(const Ray &ray, int depth) const;
};

#endif// SHADER_H
