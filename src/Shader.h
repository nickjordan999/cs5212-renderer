#ifndef SHADER_H
#define SHADER_H

#include "Camera.h"
#include "Scene.h"
#include "FrameBuffer.h"
#include "Material.h"
#include "vec.h"

// Base shader class.  The shading algorithm is determined by defaultShaderType;
// per-object overrides (material.shaderType) are always respected.
class Shader
{
public:
  virtual ~Shader() = default;

  // Trace the scene and fill the framebuffer.
  // raysPerPixel: number of rays per pixel for anti-aliasing
  void traceScene(FrameBuffer &fb, int raysPerPixel) const;

protected:
  Shader(const Scene &scene, ShaderType defaultShaderType,
         const vec3 &defaultBackground, bool shadows = true, int maxDepth = 5)
    : scene(scene),
      background(scene.getBackgroundColor().value_or(defaultBackground)),
      shadows(shadows),
      defaultShaderType(defaultShaderType),
      maxDepth(maxDepth) {}

  const Scene &scene;
  vec3 background;
  bool shadows;

private:
  ShaderType defaultShaderType;
  int maxDepth;

  vec3 shadeRay(const Ray &ray) const;
  vec3 shadeRay(const Ray &ray, int depth) const;
};

// Standard render shader - renders with material colors
class SimpleShader : public Shader
{
public:
  SimpleShader(const Scene &scene)
    : Shader(scene, ShaderType::SIMPLE, vec3(0.5f, 0.7f, 1.0f)) {}// sky blue background
};

// Normal shader - visualizes surface normals
class NormalShader : public Shader
{
public:
  NormalShader(const Scene &scene)
    : Shader(scene, ShaderType::NORMAL, vec3(0.2f, 0.2f, 0.2f)) {}// dark gray background
};

// Diffuse shader - renders with diffuse shading from lights
class DiffuseShader : public Shader
{
public:
  DiffuseShader(const Scene &scene, bool shadows = true)
    : Shader(scene, ShaderType::DIFFUSE, vec3(0.0f, 0.0f, 0.0f), shadows) {}// black background
};

// Blinn-Phong shader - ambient + diffuse + specular (halfway vector)
class BlinnPhongShader : public Shader
{
public:
  BlinnPhongShader(const Scene &scene, bool shadows = true)
    : Shader(scene, ShaderType::BLINN_PHONG, vec3(0.3f, 0.3f, 0.3f), shadows) {}
};

// Mirror shader — every surface acts as a reflective mirror.
// Material color tints the reflection; fuzziness is respected for METALLIC materials.
// maxDepth controls the maximum number of ray bounces.
class MirrorShader : public Shader
{
public:
  MirrorShader(const Scene &scene, int maxDepth = 5)
    : Shader(scene, ShaderType::MIRROR, vec3(0.5f, 0.7f, 1.0f), true, maxDepth) {}// sky blue background
};

#endif// SHADER_H
