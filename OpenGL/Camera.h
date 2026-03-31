#ifndef __CAMERA_H__
#define __CAMERA_H__

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

class Camera
{
public:
    Camera()
        : m_position(0, 0, 0), m_up(0, 1, 0), m_forward(0, 0, -1) {}

    Camera(const glm::vec3 &position, const glm::vec3 &forward, const glm::vec3 &up)
        : m_position(position), m_up(up), m_forward(glm::normalize(forward)) {}

    virtual ~Camera() {}

    void setPosition(const glm::vec3 &pos) { m_position = pos; }
    void setUp(const glm::vec3 &up) { m_up = up; }
    void setForward(const glm::vec3 &forward) { m_forward = glm::normalize(forward); }

    const glm::vec3 &position() const { return m_position; }
    const glm::vec3 &up() const { return m_up; }
    const glm::vec3 &forward() const { return m_forward; }

    glm::mat4 viewMatrix() const
    {
        return glm::lookAt(m_position, m_position + m_forward, m_up);
    }

    virtual glm::mat4 projectionMatrix() const = 0;

protected:
    glm::vec3 m_position;
    glm::vec3 m_up;
    glm::vec3 m_forward;
};

class OrthographicCamera : public Camera
{
public:
    OrthographicCamera()
        : Camera(), m_left(-7.5f), m_right(7.5f), m_bottom(-7.5f), m_top(7.5f),
          m_near(5.0f), m_far(-5.0f) {}

    OrthographicCamera(const glm::vec3 &position, const glm::vec3 &forward, const glm::vec3 &up,
                       float left, float right, float bottom, float top,
                       float near, float far)
        : Camera(position, forward, up),
          m_left(left), m_right(right), m_bottom(bottom), m_top(top),
          m_near(near), m_far(far) {}

    glm::mat4 projectionMatrix() const override
    {
        return glm::ortho(m_left, m_right, m_bottom, m_top, m_near, m_far);
    }

private:
    float m_left, m_right, m_bottom, m_top, m_near, m_far;
};

class PerspectiveCamera : public Camera
{
public:
    PerspectiveCamera()
        : Camera(glm::vec3(0, 0, 10), glm::vec3(0, 0, -1), glm::vec3(0, 1, 0)),
          m_fovY(glm::radians(45.0f)), m_aspectRatio(1.0f),
          m_near(0.1f), m_far(100.0f) {}

    PerspectiveCamera(const glm::vec3 &position, const glm::vec3 &forward, const glm::vec3 &up,
                      float fovYRadians, float aspectRatio, float near, float far)
        : Camera(position, forward, up),
          m_fovY(fovYRadians), m_aspectRatio(aspectRatio),
          m_near(near), m_far(far) {}

    void setFovY(float fovYRadians) { m_fovY = fovYRadians; }
    void setAspectRatio(float ar) { m_aspectRatio = ar; }

    glm::mat4 projectionMatrix() const override
    {
        return glm::perspective(m_fovY, m_aspectRatio, m_near, m_far);
    }

private:
    float m_fovY, m_aspectRatio, m_near, m_far;
};

#endif
