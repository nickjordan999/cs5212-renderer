#include "GLSceneRenderer.h"

#include "IcoSphere.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <stdexcept>

namespace gl_bridge {

namespace {
constexpr int kIcoSphereSubdivisions = 2;  // 320 triangles
constexpr float kDefaultShininess = 32.0f;
const glm::vec3 kDefaultAmbient(0.05f, 0.05f, 0.05f);
const glm::vec3 kDefaultSpecular(1.0f, 1.0f, 1.0f);
}  // namespace

GLSceneRenderer::GLSceneRenderer() = default;
GLSceneRenderer::~GLSceneRenderer() = default;

void GLSceneRenderer::compilePrograms()
{
  auto setup = [](Program &p, const std::string &vert, const std::string &frag) {
    p.obj.addShader(vert, sivelab::GLSLObject::VERTEX_SHADER);
    p.obj.addShader(frag, sivelab::GLSLObject::FRAGMENT_SHADER);
    p.obj.createProgram();
    // Each shader doesn't necessarily declare every uniform; createUniform returns
    // -1 for unused names which is harmless when fed to glUniform*.
    p.projLoc      = p.obj.createUniform("projMatrix");
    p.viewLoc      = p.obj.createUniform("viewMatrix");
    p.modelLoc     = p.obj.createUniform("modelMatrix");
    p.normalLoc    = p.obj.createUniform("normalMatrix");
    p.numLightsLoc        = p.obj.createUniform("numLights");
    p.lightPositionsLoc   = p.obj.createUniform("lightPositions");
    p.lightIntensitiesLoc = p.obj.createUniform("lightIntensities");
    p.cameraPosLoc = p.obj.createUniform("cameraPosWorld");
    p.diffuseLoc   = p.obj.createUniform("diffuseComponent");
    p.ambientLoc   = p.obj.createUniform("ambientComponent");
    p.specularLoc  = p.obj.createUniform("specularComponent");
    p.shininessLoc = p.obj.createUniform("shininess");
    p.patternLoc      = p.obj.createUniform("pattern");
    p.color1Loc       = p.obj.createUniform("color1");
    p.color2Loc       = p.obj.createUniform("color2");
    p.patternScaleLoc = p.obj.createUniform("patternScale");
    p.shadingModeLoc  = p.obj.createUniform("shadingMode");
  };

  setup(m_simple,     "vertexShader_simple.glsl",     "fragmentShader_simple.glsl");
  setup(m_normal,     "vertexShader_normal.glsl",     "fragmentShader_normal.glsl");
  setup(m_lambertian, "vertexShader_lambertian.glsl", "fragmentShader_lambertian.glsl");
  setup(m_blinnPhong, "vertexShader_blinnPhong.glsl", "fragmentShader_blinnPhong.glsl");
  setup(m_plane,      "vertexShader_plane.glsl",      "fragmentShader_plane.glsl");

  m_programsCompiled = true;
}

GLSceneRenderer::Program &GLSceneRenderer::programFor(GLShaderKind kind)
{
  switch (kind) {
    case GLShaderKind::Simple:     return m_simple;
    case GLShaderKind::Normal:     return m_normal;
    case GLShaderKind::Lambertian: return m_lambertian;
    case GLShaderKind::BlinnPhong: return m_blinnPhong;
  }
  return m_blinnPhong;
}

int GLSceneRenderer::planeShadingMode(GLShaderKind kind) const
{
  switch (kind) {
    case GLShaderKind::Simple:     return 0;
    case GLShaderKind::Normal:     return 1;
    case GLShaderKind::Lambertian: return 2;
    case GLShaderKind::BlinnPhong: return 3;
  }
  return 3;
}

void GLSceneRenderer::init(const Scene &scene, GLShaderKind defaultKind,
                           int viewportWidth, int viewportHeight)
{
  if (!m_programsCompiled) {
    compilePrograms();
  }

  // Background colour from scene if set; otherwise keep the default dark grey.
  if (auto bg = scene.getBackgroundColor()) {
    m_clearColor = toGlm(*bg);
  }

  // Shared icosphere mesh (unit radius); each sphere instance scales+translates.
  IcoSphereMesh ico = buildIcoSphere(kIcoSphereSubdivisions);
  m_icoSphere.upload(ico.positions, ico.normals);

  m_triangleBatches  = buildTriangleBatches(scene, defaultKind);
  m_sphereInstances  = buildSphereInstances(scene, defaultKind);
  m_planeInstances   = buildPlaneInstances(scene, defaultKind);
  m_lights           = bridgeLights(scene);

  glViewport(0, 0, viewportWidth, viewportHeight);
}

void GLSceneRenderer::bindStdUniforms(Program &p,
                                      const glm::mat4 &proj,
                                      const glm::mat4 &view,
                                      const glm::mat4 &model,
                                      const glm::mat4 &normalMat)
{
  if (p.projLoc   >= 0) glUniformMatrix4fv(p.projLoc,   1, GL_FALSE, glm::value_ptr(proj));
  if (p.viewLoc   >= 0) glUniformMatrix4fv(p.viewLoc,   1, GL_FALSE, glm::value_ptr(view));
  if (p.modelLoc  >= 0) glUniformMatrix4fv(p.modelLoc,  1, GL_FALSE, glm::value_ptr(model));
  if (p.normalLoc >= 0) glUniformMatrix4fv(p.normalLoc, 1, GL_FALSE, glm::value_ptr(normalMat));
}

void GLSceneRenderer::bindLightingUniforms(Program &p, const glm::vec3 &cameraPos)
{
  if (p.cameraPosLoc >= 0) glUniform3fv(p.cameraPosLoc, 1, glm::value_ptr(cameraPos));
  if (p.ambientLoc   >= 0) glUniform3fv(p.ambientLoc,   1, glm::value_ptr(kDefaultAmbient));
  if (p.specularLoc  >= 0) glUniform3fv(p.specularLoc,  1, glm::value_ptr(kDefaultSpecular));
  if (p.shininessLoc >= 0) glUniform1f(p.shininessLoc, kDefaultShininess);

  // Pack scene lights (truncated by bridgeLights() to kMaxLights) into contiguous
  // arrays. If there are no scene lights, fall back to a unit-intensity light at
  // the camera position so unlit scenes still show something.
  glm::vec3 positions[kMaxLights];
  glm::vec3 intensities[kMaxLights];
  int count = 0;
  if (m_lights.empty()) {
    positions[0]   = cameraPos;
    intensities[0] = glm::vec3(1.0f);
    count = 1;
  } else {
    for (const auto &L : m_lights) {
      if (count >= kMaxLights) break;
      positions[count]   = L.position;
      intensities[count] = L.intensity;
      ++count;
    }
  }
  if (p.numLightsLoc        >= 0) glUniform1i(p.numLightsLoc, count);
  if (p.lightPositionsLoc   >= 0) glUniform3fv(p.lightPositionsLoc,   count, glm::value_ptr(positions[0]));
  if (p.lightIntensitiesLoc >= 0) glUniform3fv(p.lightIntensitiesLoc, count, glm::value_ptr(intensities[0]));
}

void GLSceneRenderer::render(const GLCamera &camera)
{
  glClearColor(m_clearColor.r, m_clearColor.g, m_clearColor.b, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glm::mat4 proj  = camera.projectionMatrix();
  glm::mat4 view  = camera.viewMatrix();
  glm::vec3 camPos = camera.position();

  glm::mat4 identity(1.0f);

  // ------------ Triangle batches ------------
  for (auto &batch : m_triangleBatches) {
    Program &p = programFor(batch.shaderKind);
    p.obj.activate();
    bindStdUniforms(p, proj, view, identity, identity);
    bindLightingUniforms(p, camPos);
    if (p.diffuseLoc >= 0) glUniform3fv(p.diffuseLoc, 1, glm::value_ptr(batch.color));
    batch.mesh.draw();
    p.obj.deactivate();
  }

  // ------------ Spheres (shared icosphere mesh, per-instance model + color) ------------
  for (auto &inst : m_sphereInstances) {
    Program &p = programFor(inst.shaderKind);
    p.obj.activate();
    glm::mat4 normalMat = glm::transpose(glm::inverse(inst.modelMatrix));
    bindStdUniforms(p, proj, view, inst.modelMatrix, normalMat);
    bindLightingUniforms(p, camPos);
    if (p.diffuseLoc >= 0) glUniform3fv(p.diffuseLoc, 1, glm::value_ptr(inst.color));
    m_icoSphere.draw();
    p.obj.deactivate();
  }

  // ------------ Planes (dedicated shader handles SOLID/CHECKER/HEX + lighting modes) ------------
  for (auto &pl : m_planeInstances) {
    Program &p = m_plane;
    p.obj.activate();
    bindStdUniforms(p, proj, view, identity, identity);
    bindLightingUniforms(p, camPos);

    if (p.patternLoc      >= 0) glUniform1i(p.patternLoc, pl.pattern);
    if (p.color1Loc       >= 0) glUniform3fv(p.color1Loc, 1, glm::value_ptr(pl.color1));
    if (p.color2Loc       >= 0) glUniform3fv(p.color2Loc, 1, glm::value_ptr(pl.color2));
    if (p.patternScaleLoc >= 0) glUniform1f(p.patternScaleLoc, pl.patternScale);
    if (p.shadingModeLoc  >= 0) glUniform1i(p.shadingModeLoc, planeShadingMode(pl.shaderKind));

    pl.quad.draw();
    p.obj.deactivate();
  }
}

}  // namespace gl_bridge
