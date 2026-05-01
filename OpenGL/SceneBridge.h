#ifndef OPENGL_SCENEBRIDGE_H
#define OPENGL_SCENEBRIDGE_H

#include <memory>
#include <vector>

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"

// OpenGL camera classes are GLCamera/GLOrthographicCamera/GLPerspectiveCamera
// to avoid colliding with src/Camera.h::Camera (raytracer).
#include "Camera.h"

// Raytracer types — reachable via the cs4212-util include path (src/).
#include "Scene.h"
#include "Material.h"
#include "vec.h"

#include "GLMesh.h"

namespace gl_bridge {

constexpr int kMaxLights = 4;

enum class GLShaderKind {
  Simple,
  Normal,
  Lambertian,
  BlinnPhong
};

struct GLLight
{
  glm::vec3 position;
  glm::vec3 intensity;
};

struct TriangleBatch
{
  GLMesh mesh;
  glm::vec3 color;
  GLShaderKind shaderKind;
};

struct SphereInstance
{
  glm::mat4 modelMatrix;
  glm::vec3 color;
  GLShaderKind shaderKind;
};

struct PlaneInstance
{
  GLMesh quad;
  int pattern;            // 0 = SOLID, 1 = CHECKER, 2 = HEX
  glm::vec3 color1;
  glm::vec3 color2;
  float patternScale;
  GLShaderKind shaderKind;
};

glm::vec3 toGlm(const vec3 &v);

// fovY = 2 * atan(1 / focal_length); up = (0, 1, 0).
// rtCam must be a PerspectiveBasicCamera (the only Camera type the raytracer constructs).
std::unique_ptr<GLPerspectiveCamera> bridgeCamera(const ::Camera &rtCam, float aspectRatio);

std::vector<GLLight> bridgeLights(const Scene &scene);

GLShaderKind pickShaderKind(const Material &mat, GLShaderKind defaultKind);

std::vector<TriangleBatch> buildTriangleBatches(const Scene &scene, GLShaderKind defaultKind);
std::vector<SphereInstance> buildSphereInstances(const Scene &scene, GLShaderKind defaultKind);
std::vector<PlaneInstance>  buildPlaneInstances(const Scene &scene, GLShaderKind defaultKind);

}  // namespace gl_bridge

#endif  // OPENGL_SCENEBRIDGE_H
