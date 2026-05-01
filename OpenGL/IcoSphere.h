#ifndef OPENGL_ICOSPHERE_H
#define OPENGL_ICOSPHERE_H

#include <vector>

namespace gl_bridge {

// Mesh data for a unit-radius icosphere centered at the origin.
// positions[i*3 + 0..2] is one vertex; normals[i*3 + 0..2] is its normal.
// For a unit sphere, normals == positions.
struct IcoSphereMesh
{
  std::vector<float> positions;
  std::vector<float> normals;
};

// Generate an icosphere by recursive midpoint subdivision of an icosahedron.
//   subdivisions = 0 → 20 triangles (raw icosahedron, normalized)
//   subdivisions = 2 → 320 triangles (default)
IcoSphereMesh buildIcoSphere(int subdivisions);

}  // namespace gl_bridge

#endif  // OPENGL_ICOSPHERE_H
