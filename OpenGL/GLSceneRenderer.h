#ifndef OPENGL_GLSCENERENDERER_H
#define OPENGL_GLSCENERENDERER_H

#include "GLMesh.h"
#include "GLSL.h"
#include "SceneBridge.h"

#include <memory>
#include <optional>
#include <vector>

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"

namespace gl_bridge {

// Owns all GPU resources and per-frame draw logic for rendering a Scene.
//
// Lifecycle:
//   1. Construct a default GLSceneRenderer (no GPU work yet).
//   2. After a GL context is current, call init(scene, defaultKind, w, h).
//   3. Per frame, call render(camera) — clears, draws all batches.
class GLSceneRenderer
{
public:
  GLSceneRenderer();
  ~GLSceneRenderer();

  GLSceneRenderer(const GLSceneRenderer &) = delete;
  GLSceneRenderer &operator=(const GLSceneRenderer &) = delete;

  void init(const Scene &scene, GLShaderKind defaultKind, int viewportWidth, int viewportHeight);

  void setClearColor(const glm::vec3 &color) { m_clearColor = color; }

  // Draw one frame: clear + all batches + spheres + planes, in that order.
  void render(const GLCamera &camera);

private:
  struct Program
  {
    sivelab::GLSLObject obj;
    // Standard transforms (present in every shader).
    int projLoc      = -1;
    int viewLoc      = -1;
    int modelLoc     = -1;
    int normalLoc    = -1;
    // Lighting.
    int numLightsLoc        = -1;  // int
    int lightPositionsLoc   = -1;  // vec3[MAX_LIGHTS]
    int lightIntensitiesLoc = -1;  // vec3[MAX_LIGHTS]
    int cameraPosLoc        = -1;
    int diffuseLoc          = -1;
    int ambientLoc          = -1;
    int specularLoc         = -1;
    int shininessLoc        = -1;
    // Plane-only.
    int patternLoc       = -1;
    int color1Loc        = -1;
    int color2Loc        = -1;
    int patternScaleLoc  = -1;
    int shadingModeLoc   = -1;
  };

  void compilePrograms();
  Program &programFor(GLShaderKind kind);
  int planeShadingMode(GLShaderKind kind) const;

  void bindStdUniforms(Program &p,
                       const glm::mat4 &proj,
                       const glm::mat4 &view,
                       const glm::mat4 &model,
                       const glm::mat4 &normalMat);
  void bindLightingUniforms(Program &p,
                            const glm::vec3 &cameraPos);

  // Programs.
  Program m_simple;
  Program m_normal;
  Program m_lambertian;
  Program m_blinnPhong;
  Program m_plane;
  bool m_programsCompiled = false;

  // GPU geometry.
  GLMesh m_icoSphere;

  // Bridged scene data.
  std::vector<TriangleBatch>  m_triangleBatches;
  std::vector<SphereInstance> m_sphereInstances;
  std::vector<PlaneInstance>  m_planeInstances;
  std::vector<GLLight>        m_lights;

  glm::vec3 m_clearColor = glm::vec3(0.2f, 0.2f, 0.2f);
};

}  // namespace gl_bridge

#endif  // OPENGL_GLSCENERENDERER_H
