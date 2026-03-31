#include <cstdlib>
#include <iostream>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "GLSL.h"
#include "Camera.h"

int CheckGLErrors(const char *s)
{
    int errCount = 0;
    return errCount;
}

int main(void)
{
    /* Initialize the library */
    if (!glfwInit()) {
        exit (-1);
    }
    // throw std::runtime_error("Error! initialization of glfw failed!");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    /* Create a windowed mode window and its OpenGL context */
    int winWidth = 1000;
    float aspectRatio = 1.0; // 16.0 / 9.0; // winWidth / (float)winHeight;
    int winHeight = winWidth / aspectRatio;
    
    GLFWwindow* window = glfwCreateWindow(winWidth, winHeight, "GLFW Example", NULL, NULL);
    if (!window) {
        std::cerr << "GLFW did not create a window!" << std::endl;
        
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    GLenum err=glewInit();

    if(err != GLEW_OK) {
        std::cerr <<"GLEW Error! glewInit failed, exiting."<< std::endl;
        exit(EXIT_FAILURE);
    }

    const GLubyte* renderer = glGetString (GL_RENDERER);
    const GLubyte* version = glGetString (GL_VERSION);
    std::cout << "Renderer: " << renderer << std::endl;
    std::cout << "OpenGL version supported: " << version << std::endl;

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClearColor(0.2, 0.2, 0.2, 1.0); // Background Dark Grey

    int fb_width, fb_height;
    glfwGetFramebufferSize(window, &fb_width, &fb_height);
    glViewport(0, 0, fb_width, fb_height);

    GLint major_version;
    glGetIntegerv(GL_MAJOR_VERSION, &major_version);
    std::cout << "GL_MAJOR_VERSION: " << major_version << std::endl;

    GLuint m_triangleVBO[2];
    GLuint m_VAO;

    // create a Vertex Array Buffer to hold our triangle data
    glGenBuffers(2, m_triangleVBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_triangleVBO[0]);

    // this is the actual triangle data that will be copied to
    // the GPU memory
    std::vector< float > host_VertexBuffer{ -3.0f, -3.0f, 0.0f,    // V0
                                            3.0f, -3.0f, 0.0f,    // V1
                                            0.0f, 3.0f, 0.0f };   // V2

    int numBytes = host_VertexBuffer.size() * sizeof(float);

    // copy the numBytes from host_VertexBuffer t the GPU and store in
    // the currently bound VBO
    glBufferData(GL_ARRAY_BUFFER, numBytes, host_VertexBuffer.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // once copied, we no longer need the data on the host
    host_VertexBuffer.clear();

    // Create color buffer and upload color data
    glBindBuffer(GL_ARRAY_BUFFER, m_triangleVBO[1]);
    std::vector< float > host_ColorBuffer{ 0.5f, 0.0f, 0.5f,    // V0 - Purple
                                           0.0f, 1.0f, 0.0f,    // V1 - Green
                                           0.0f, 0.0f, 1.0f };   // V2 - Blue

    int colorNumBytes = host_ColorBuffer.size() * sizeof(float);
    glBufferData(GL_ARRAY_BUFFER, colorNumBytes, host_ColorBuffer.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    host_ColorBuffer.clear();

    // create a vertex array object that will map the attributes in
    // our vertex buffer to different location attributes for our
    // shaders
    glGenVertexArrays(1, &m_VAO);
    glBindVertexArray(m_VAO);

    // VAO details - Location 0: Position
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, m_triangleVBO[0]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);

    // VAO details - Location 1: Color
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, m_triangleVBO[1]);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);

    glBindVertexArray(0);

    // Create a shader using my GLSLObject class                                                            
    sivelab::GLSLObject shader;
    shader.addShader( "vertexShader_withMatrixTransformation.glsl", sivelab::GLSLObject::VERTEX_SHADER );
    shader.addShader( "fragmentShader_passthrough.glsl", sivelab::GLSLObject::FRAGMENT_SHADER );
    shader.createProgram();

    GLuint projMatrixID, viewMatrixID, modelMatrixID;
    projMatrixID = shader.createUniform( "projMatrix" );
    viewMatrixID = shader.createUniform( "viewMatrix" );
    modelMatrixID = shader.createUniform( "modelMatrix" );

    // Toggle: false = orthographic, true = perspective
    bool usePerspective = true;

    OrthographicCamera orthoCam(
        glm::vec3(0, 0, 0), glm::vec3(0, 0, -1), glm::vec3(0, 1, 0),
        -7.5f, 7.5f, -7.5f, 7.5f, 5.0f, -5.0f);

    PerspectiveCamera perspCam(
        glm::vec3(0, 0, 10), glm::vec3(0, 0, -1), glm::vec3(0, 1, 0),
        glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);

    Camera *activeCamera = usePerspective ? (Camera *)&perspCam : (Camera *)&orthoCam;

    // Model transform state
    float modelRotation = 0.0f;  // radians around Z axis
    float modelScale = 1.0f;

    double timeDiff = 0.0, startFrameTime = 0.0, endFrameTime = 0.0;

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        endFrameTime = glfwGetTime();
        timeDiff = endFrameTime - startFrameTime;
        startFrameTime = glfwGetTime();

        // Clear the window's buffer (or clear the screen to our
        // background color)
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 M_view = activeCamera->viewMatrix();
        glm::mat4 M_proj = activeCamera->projectionMatrix();

        /* Render your objects here */
        shader.activate();

        // Build model matrix: scale then rotate around Z
        glm::mat4 M_model = glm::rotate(glm::mat4(1.0f), modelRotation, glm::vec3(0.0f, 0.0f, 1.0f));
        M_model = glm::scale(M_model, glm::vec3(modelScale));

        glUniformMatrix4fv(projMatrixID, 1, GL_FALSE, glm::value_ptr( M_proj ));
        glUniformMatrix4fv(viewMatrixID, 1, GL_FALSE, glm::value_ptr( M_view ));
        glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, glm::value_ptr( M_model ));

        glBindVertexArray(m_VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);

        shader.deactivate();

        // Swap the front and back buffers
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();

        float moveRatePerFrame = 0.05;
        glm::vec3 camPos = activeCamera->position();
        glm::vec3 camFwd = activeCamera->forward();
        glm::vec3 camRight = glm::normalize(glm::cross(camFwd, activeCamera->up()));

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
          camPos += camFwd * moveRatePerFrame;
        }
        else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
          camPos -= camRight * moveRatePerFrame;
        }
        else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
          camPos -= camFwd * moveRatePerFrame;
        }
        else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
          camPos += camRight * moveRatePerFrame;
        }
        activeCamera->setPosition(camPos);

        // Model rotation: J = CCW, K = CW
        float rotateRate = 1.0f;  // radians per second
        if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) {
            modelRotation += rotateRate * (float)timeDiff;
        }
        if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) {
            modelRotation -= rotateRate * (float)timeDiff;
        }

        // Model scale: N = scale down, M = scale up
        float scaleRate = 1.0f;  // per second
        if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS) {
            modelScale -= scaleRate * (float)timeDiff;
            if (modelScale < 0.1f) modelScale = 0.1f;
        }
        if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) {
            modelScale += scaleRate * (float)timeDiff;
        }

        if (glfwGetKey( window, GLFW_KEY_T ) == GLFW_PRESS) {
            std::cout << "fps: " << 1.0/timeDiff << std::endl;
        }
        if (glfwGetKey( window, GLFW_KEY_ESCAPE ) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, 1);
        }
    }
  
    glfwTerminate();
    return 0;
}
