#include "GLMesh.h"

#include <stdexcept>
#include <utility>

namespace gl_bridge {

GLMesh::GLMesh() = default;

GLMesh::GLMesh(const std::vector<float> &positions, const std::vector<float> &normals)
{
  upload(positions, normals);
}

GLMesh::~GLMesh()
{
  release();
}

GLMesh::GLMesh(GLMesh &&other) noexcept
  : m_vao(other.m_vao),
    m_vboPositions(other.m_vboPositions),
    m_vboNormals(other.m_vboNormals),
    m_vertexCount(other.m_vertexCount)
{
  other.m_vao = 0;
  other.m_vboPositions = 0;
  other.m_vboNormals = 0;
  other.m_vertexCount = 0;
}

GLMesh &GLMesh::operator=(GLMesh &&other) noexcept
{
  if (this != &other) {
    release();
    m_vao = other.m_vao;
    m_vboPositions = other.m_vboPositions;
    m_vboNormals = other.m_vboNormals;
    m_vertexCount = other.m_vertexCount;
    other.m_vao = 0;
    other.m_vboPositions = 0;
    other.m_vboNormals = 0;
    other.m_vertexCount = 0;
  }
  return *this;
}

void GLMesh::release()
{
  if (m_vboPositions) glDeleteBuffers(1, &m_vboPositions);
  if (m_vboNormals)   glDeleteBuffers(1, &m_vboNormals);
  if (m_vao)          glDeleteVertexArrays(1, &m_vao);
  m_vao = m_vboPositions = m_vboNormals = 0;
  m_vertexCount = 0;
}

void GLMesh::upload(const std::vector<float> &positions, const std::vector<float> &normals)
{
  if (positions.size() != normals.size()) {
    throw std::runtime_error("GLMesh: positions and normals must have equal length");
  }
  if (positions.size() % 3 != 0) {
    throw std::runtime_error("GLMesh: vertex stream length must be a multiple of 3");
  }

  if (!m_vao) {
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vboPositions);
    glGenBuffers(1, &m_vboNormals);
  }

  glBindVertexArray(m_vao);

  glBindBuffer(GL_ARRAY_BUFFER, m_vboPositions);
  glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(float),
               positions.data(), GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), nullptr);

  glBindBuffer(GL_ARRAY_BUFFER, m_vboNormals);
  glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float),
               normals.data(), GL_STATIC_DRAW);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), nullptr);

  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  m_vertexCount = static_cast<GLsizei>(positions.size() / 3);
}

void GLMesh::draw() const
{
  if (!m_vao || m_vertexCount == 0) return;
  glBindVertexArray(m_vao);
  glDrawArrays(GL_TRIANGLES, 0, m_vertexCount);
  glBindVertexArray(0);
}

}  // namespace gl_bridge
