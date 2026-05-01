#include "IcoSphere.h"

#include "Solid.h"

#include <array>
#include <cmath>

namespace gl_bridge {

namespace {

struct Tri
{
  vec3 a, b, c;
};

vec3 unitMidpoint(const vec3 &p, const vec3 &q)
{
  return ((p + q) * 0.5f).normalized();
}

}  // namespace

IcoSphereMesh buildIcoSphere(int subdivisions)
{
  // Seed: the canonical icosahedron from Solid.h, vertices renormalized to the
  // unit sphere so subdivision-midpoints are also on the unit sphere.
  Solid seed = Solid::Icosahedron();
  const auto &seedVerts = seed.getVertices();
  const auto &seedFaces = seed.getFaces();

  std::vector<vec3> verts;
  verts.reserve(seedVerts.size());
  for (const auto &v : seedVerts) {
    verts.push_back(v.normalized());
  }

  std::vector<Tri> tris;
  tris.reserve(seed.faceCount());
  for (int i = 0; i < seed.faceCount(); ++i) {
    tris.push_back({ verts[seedFaces[i * 3 + 0]],
                     verts[seedFaces[i * 3 + 1]],
                     verts[seedFaces[i * 3 + 2]] });
  }

  // Each subdivision replaces every triangle with 4: (a,m_ab,m_ca), (m_ab,b,m_bc),
  // (m_ca,m_bc,c), (m_ab,m_bc,m_ca). Midpoints renormalize to the unit sphere.
  for (int level = 0; level < subdivisions; ++level) {
    std::vector<Tri> next;
    next.reserve(tris.size() * 4);
    for (const auto &t : tris) {
      vec3 m_ab = unitMidpoint(t.a, t.b);
      vec3 m_bc = unitMidpoint(t.b, t.c);
      vec3 m_ca = unitMidpoint(t.c, t.a);
      next.push_back({ t.a,  m_ab, m_ca });
      next.push_back({ m_ab, t.b,  m_bc });
      next.push_back({ m_ca, m_bc, t.c  });
      next.push_back({ m_ab, m_bc, m_ca });
    }
    tris.swap(next);
  }

  IcoSphereMesh mesh;
  mesh.positions.reserve(tris.size() * 9);
  mesh.normals.reserve(tris.size() * 9);
  auto push = [&](const vec3 &p) {
    mesh.positions.push_back(p[0]);
    mesh.positions.push_back(p[1]);
    mesh.positions.push_back(p[2]);
    mesh.normals.push_back(p[0]);
    mesh.normals.push_back(p[1]);
    mesh.normals.push_back(p[2]);
  };
  for (const auto &t : tris) {
    push(t.a);
    push(t.b);
    push(t.c);
  }
  return mesh;
}

}  // namespace gl_bridge
