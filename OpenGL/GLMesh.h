#ifndef OPENGL_GLMESH_H
#define OPENGL_GLMESH_H

#include <vector>

#include <GL/glew.h>

namespace gl_bridge {

// RAII wrapper around a VAO + position VBO + normal VBO. Bind once at
// construction; draw() binds the VAO and issues glDrawArrays(GL_TRIANGLES).
//
// Vertex layout (matches the existing shaders):
//   layout(location=0) in vec3 in_Position;
//   layout(location=1) in vec3 in_Normal;
class GLMesh
{
public:
  GLMesh();
  GLMesh(const std::vector<float> &positions, const std::vector<float> &normals);
  ~GLMesh();

  GLMesh(const GLMesh &) = delete;
  GLMesh &operator=(const GLMesh &) = delete;

  GLMesh(GLMesh &&other) noexcept;
  GLMesh &operator=(GLMesh &&other) noexcept;

  // Reupload data into this mesh (creates GL objects on first call).
  void upload(const std::vector<float> &positions, const std::vector<float> &normals);

  // Draws GL_TRIANGLES for the previously uploaded vertex count.
  void draw() const;

  GLsizei vertexCount() const { return m_vertexCount; }

private:
  void release();

  GLuint m_vao = 0;
  GLuint m_vboPositions = 0;
  GLuint m_vboNormals = 0;
  GLsizei m_vertexCount = 0;
};

}  // namespace gl_bridge

#endif  // OPENGL_GLMESH_H
