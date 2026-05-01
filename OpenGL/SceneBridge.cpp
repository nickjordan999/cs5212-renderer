#include "SceneBridge.h"

#include "Camera.h"          // src/Camera.h is reachable via cs4212-util's interface include path
                             // but here we only need the OpenGL Camera; the raytracer's Camera comes
                             // through Scene.h's include chain. We dynamic_cast on it below.
#include "Sphere.h"
#include "Triangle.h"
#include "Plane.h"
#include "Light.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cmath>
#include <stdexcept>

namespace gl_bridge {

glm::vec3 toGlm(const vec3 &v)
{
  return glm::vec3(v[0], v[1], v[2]);
}

std::unique_ptr<GLPerspectiveCamera>
bridgeCamera(const ::Camera &rtCam, float aspectRatio)
{
  const auto *persp = dynamic_cast<const PerspectiveBasicCamera *>(&rtCam);
  if (!persp) {
    throw std::runtime_error("bridgeCamera: only PerspectiveBasicCamera is supported");
  }

  glm::vec3 pos     = toGlm(persp->getPosition());
  glm::vec3 forward = glm::normalize(toGlm(persp->getDirection()));
  glm::vec3 up      = glm::vec3(0.0f, 1.0f, 0.0f);
  float fovY        = 2.0f * std::atan(1.0f / persp->getFocalLength());

  return std::make_unique<GLPerspectiveCamera>(
      pos, forward, up, fovY, aspectRatio, 0.1f, 1000.0f);
}

std::vector<GLLight> bridgeLights(const Scene &scene)
{
  std::vector<GLLight> out;
  const auto &rtLights = scene.getLights();
  out.reserve(rtLights.size());
  for (const auto &L : rtLights) {
    out.push_back({ toGlm(L.getPosition()), toGlm(L.getIntensity()) });
    if (static_cast<int>(out.size()) >= kMaxLights) break;
  }
  return out;
}

GLShaderKind pickShaderKind(const Material &mat, GLShaderKind defaultKind)
{
  if (!mat.shaderType.has_value()) return defaultKind;
  switch (*mat.shaderType) {
    case ShaderType::SIMPLE:       return GLShaderKind::Simple;
    case ShaderType::NORMAL:       return GLShaderKind::Normal;
    case ShaderType::DIFFUSE:      return GLShaderKind::Lambertian;
    case ShaderType::BLINN_PHONG:  return GLShaderKind::BlinnPhong;
    // Unsupported in OpenGL pipeline — fall back to Blinn-Phong with the material's color.
    case ShaderType::MIRROR:
    case ShaderType::DIELECTRIC:
    case ShaderType::PATH_DIFFUSE:
    default:                       return GLShaderKind::BlinnPhong;
  }
}

namespace {

// Group key for batching triangles: (color, shader kind). We compare colors at
// 1/255 resolution so small float jitter doesn't split batches.
struct BatchKey
{
  int r, g, b;
  GLShaderKind kind;
  bool operator==(const BatchKey &o) const
  {
    return r == o.r && g == o.g && b == o.b && kind == o.kind;
  }
};

BatchKey makeKey(const glm::vec3 &c, GLShaderKind k)
{
  auto q = [](float v) { return static_cast<int>(std::round(v * 255.0f)); };
  return { q(c.x), q(c.y), q(c.z), k };
}

}  // namespace

std::vector<TriangleBatch>
buildTriangleBatches(const Scene &scene, GLShaderKind defaultKind)
{
  // Bucket triangles by (color, shader kind).
  struct Bucket
  {
    std::vector<float> positions;
    std::vector<float> normals;
    glm::vec3 color;
    GLShaderKind kind;
  };
  std::vector<Bucket> buckets;

  auto findOrAdd = [&](const glm::vec3 &color, GLShaderKind kind) -> Bucket & {
    BatchKey key = makeKey(color, kind);
    for (auto &b : buckets) {
      if (makeKey(b.color, b.kind) == key) return b;
    }
    buckets.push_back({ {}, {}, color, kind });
    return buckets.back();
  };

  for (const auto &tri : scene.getTriangles()) {
    const Material &mat = tri.getMaterial();
    GLShaderKind kind = pickShaderKind(mat, defaultKind);
    glm::vec3 color = toGlm(mat.color);
    Bucket &bk = findOrAdd(color, kind);

    glm::vec3 n = glm::normalize(toGlm(tri.getNormal()));
    glm::vec3 v0 = toGlm(tri.getV0());
    glm::vec3 v1 = toGlm(tri.getV1());
    glm::vec3 v2 = toGlm(tri.getV2());

    auto pushVN = [&](const glm::vec3 &p, const glm::vec3 &nn) {
      bk.positions.push_back(p.x); bk.positions.push_back(p.y); bk.positions.push_back(p.z);
      bk.normals.push_back(nn.x); bk.normals.push_back(nn.y); bk.normals.push_back(nn.z);
    };
    pushVN(v0, n); pushVN(v1, n); pushVN(v2, n);
  }

  std::vector<TriangleBatch> out;
  out.reserve(buckets.size());
  for (auto &b : buckets) {
    TriangleBatch tb;
    tb.mesh.upload(b.positions, b.normals);
    tb.color = b.color;
    tb.shaderKind = b.kind;
    out.push_back(std::move(tb));
  }
  return out;
}

std::vector<SphereInstance>
buildSphereInstances(const Scene &scene, GLShaderKind defaultKind)
{
  std::vector<SphereInstance> out;
  const auto &spheres = scene.getSpheres();
  out.reserve(spheres.size());
  for (const auto &s : spheres) {
    SphereInstance inst;
    glm::mat4 M(1.0f);
    M = glm::translate(M, toGlm(s.getCenter()));
    M = glm::scale(M, glm::vec3(s.getRadius()));
    inst.modelMatrix = M;
    inst.color = toGlm(s.getMaterial().color);
    inst.shaderKind = pickShaderKind(s.getMaterial(), defaultKind);
    out.push_back(std::move(inst));
  }
  return out;
}

namespace {

// Build a 100x100 quad oriented to the plane normal at its `point`.
// Tangent T and bitangent B come from Gram-Schmidt against the world axis least
// parallel to N. The resulting two triangles cover [-50, 50] in T and B.
void buildPlaneQuad(const glm::vec3 &point, const glm::vec3 &normal,
                    std::vector<float> &positions, std::vector<float> &normals)
{
  glm::vec3 N = glm::normalize(normal);
  glm::vec3 ref = (std::abs(N.x) < 0.9f) ? glm::vec3(1, 0, 0) : glm::vec3(0, 1, 0);
  glm::vec3 T = glm::normalize(ref - N * glm::dot(ref, N));
  glm::vec3 B = glm::normalize(glm::cross(N, T));

  const float S = 50.0f;
  glm::vec3 v00 = point + (-T * S) + (-B * S);
  glm::vec3 v10 = point + ( T * S) + (-B * S);
  glm::vec3 v11 = point + ( T * S) + ( B * S);
  glm::vec3 v01 = point + (-T * S) + ( B * S);

  auto push = [&](const glm::vec3 &p) {
    positions.push_back(p.x); positions.push_back(p.y); positions.push_back(p.z);
    normals.push_back(N.x);   normals.push_back(N.y);   normals.push_back(N.z);
  };
  // Two triangles, winding consistent with N.
  push(v00); push(v10); push(v11);
  push(v00); push(v11); push(v01);
}

}  // namespace

std::vector<PlaneInstance>
buildPlaneInstances(const Scene &scene, GLShaderKind defaultKind)
{
  std::vector<PlaneInstance> out;
  const auto &planes = scene.getPlanes();
  out.reserve(planes.size());
  for (const auto &p : planes) {
    PlaneInstance inst;
    std::vector<float> positions;
    std::vector<float> normals;
    buildPlaneQuad(toGlm(p.getPoint()), toGlm(p.getNormal()), positions, normals);
    inst.quad.upload(positions, normals);
    inst.pattern = static_cast<int>(p.getPattern());
    inst.color1 = toGlm(p.getMat1().color);
    inst.color2 = toGlm(p.getMat2().color);
    inst.patternScale = p.getScale();
    inst.shaderKind = pickShaderKind(p.getMat1(), defaultKind);
    out.push_back(std::move(inst));
  }
  return out;
}

}  // namespace gl_bridge
