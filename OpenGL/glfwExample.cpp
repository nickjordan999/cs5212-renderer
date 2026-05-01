#include <cstdlib>
#include <cmath>
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

    // Build an icosahedron (12 verts, 20 faces) with flat per-face normals.
    const float phi = (1.0f + std::sqrt(5.0f)) / 2.0f;
    std::vector<glm::vec3> icoVerts = {
        {-1.0f,  phi,  0.0f}, { 1.0f,  phi,  0.0f},
        {-1.0f, -phi,  0.0f}, { 1.0f, -phi,  0.0f},
        { 0.0f, -1.0f,  phi}, { 0.0f,  1.0f,  phi},
        { 0.0f, -1.0f, -phi}, { 0.0f,  1.0f, -phi},
        { phi,  0.0f, -1.0f}, { phi,  0.0f,  1.0f},
        {-phi,  0.0f, -1.0f}, {-phi,  0.0f,  1.0f}
    };
    for (auto &v : icoVerts) v = glm::normalize(v);

    const int icoFaces[20][3] = {
        {0,11,5}, {0,5,1},  {0,1,7},   {0,7,10}, {0,10,11},
        {1,5,9},  {5,11,4}, {11,10,2}, {10,7,6}, {7,1,8},
        {3,9,4},  {3,4,2},  {3,2,6},   {3,6,8},  {3,8,9},
        {4,9,5},  {2,4,11}, {6,2,10},  {8,6,7},  {9,8,1}
    };

    const float icoScale = 3.0f;
    std::vector<float> host_VertexBuffer;
    std::vector<float> host_NormalBuffer;
    host_VertexBuffer.reserve(20 * 3 * 3);
    host_NormalBuffer.reserve(20 * 3 * 3);
    for (int f = 0; f < 20; ++f) {
        glm::vec3 a = icoVerts[icoFaces[f][0]] * icoScale;
        glm::vec3 b = icoVerts[icoFaces[f][1]] * icoScale;
        glm::vec3 c = icoVerts[icoFaces[f][2]] * icoScale;
        glm::vec3 n = glm::normalize(glm::cross(b - a, c - a));
        glm::vec3 tri[3] = { a, b, c };
        for (int i = 0; i < 3; ++i) {
            host_VertexBuffer.push_back(tri[i].x);
            host_VertexBuffer.push_back(tri[i].y);
            host_VertexBuffer.push_back(tri[i].z);
            host_NormalBuffer.push_back(n.x);
            host_NormalBuffer.push_back(n.y);
            host_NormalBuffer.push_back(n.z);
        }
    }

    int numBytes = host_VertexBuffer.size() * sizeof(float);
    glBufferData(GL_ARRAY_BUFFER, numBytes, host_VertexBuffer.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    host_VertexBuffer.clear();

    glBindBuffer(GL_ARRAY_BUFFER, m_triangleVBO[1]);
    int normalNumBytes = host_NormalBuffer.size() * sizeof(float);
    glBufferData(GL_ARRAY_BUFFER, normalNumBytes, host_NormalBuffer.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    host_NormalBuffer.clear();

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
    shader.addShader( "vertexShader_normal.glsl", sivelab::GLSLObject::VERTEX_SHADER );
    shader.addShader( "fragmentShader_normal.glsl", sivelab::GLSLObject::FRAGMENT_SHADER );
    shader.createProgram();

    GLuint projMatrixID, viewMatrixID, modelMatrixID, normalMatrixID, lightPosWorldID;
    GLuint diffuseComponentID, specularComponentID, ambientComponentID, shininessID, cameraPosWorldID;
    projMatrixID = shader.createUniform( "projMatrix" );
    viewMatrixID = shader.createUniform( "viewMatrix" );
    modelMatrixID = shader.createUniform( "modelMatrix" );
    normalMatrixID = shader.createUniform( "normalMatrix");
    lightPosWorldID = shader.createUniform( "lightPosWorld" );
    diffuseComponentID = shader.createUniform( "diffuseComponent" );
    specularComponentID = shader.createUniform( "specularComponent" );
    ambientComponentID = shader.createUniform( "ambientComponent" );
    shininessID = shader.createUniform( "shininess" );
    cameraPosWorldID = shader.createUniform( "cameraPosWorld" );

    // Toggle: false = orthographic, true = perspective
    bool usePerspective = true;

    GLOrthographicCamera orthoCam(
        glm::vec3(0, 0, 0), glm::vec3(0, 0, -1), glm::vec3(0, 1, 0),
        -7.5f, 7.5f, -7.5f, 7.5f, 5.0f, -5.0f);

    GLPerspectiveCamera perspCam(
        glm::vec3(0, 0, 10), glm::vec3(0, 0, -1), glm::vec3(0, 1, 0),
        glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);

    GLCamera *activeCamera = usePerspective ? (GLCamera *)&perspCam : (GLCamera *)&orthoCam;

    // Model transform state
    float modelRotation = 0.0f;   // radians around Z axis
    float modelRotationX = 0.0f;  // radians around X axis
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

        // Build model matrix: scale, then rotate around X, then rotate around Z
        glm::mat4 M_model = glm::rotate(glm::mat4(1.0f), modelRotation, glm::vec3(0.0f, 0.0f, 1.0f));
        M_model = glm::rotate(M_model, modelRotationX, glm::vec3(1.0f, 0.0f, 0.0f));
        M_model = glm::scale(M_model, glm::vec3(modelScale));

        glm::mat4 M_normal = glm::transpose(glm::inverse(M_model));

        glUniformMatrix4fv(projMatrixID, 1, GL_FALSE, glm::value_ptr( M_proj ));
        glUniformMatrix4fv(viewMatrixID, 1, GL_FALSE, glm::value_ptr( M_view ));
        glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, glm::value_ptr( M_model ));
        glUniformMatrix4fv(normalMatrixID, 1, GL_FALSE, glm::value_ptr( M_normal ));

        glm::vec4 lightPosWorld(0.0f, 0.0f, 5.0f, 1.0f);

        glUniform4fv(lightPosWorldID, 1, glm::value_ptr(lightPosWorld));

        glm::vec3 diffuseComponent(1.0f, 0.0f, 0.0f);
        glm::vec3 specularComponent(1.0f, 1.0f, 1.0f);
        glm::vec3 ambientComponent(0.05f, 0.05f, 0.05f);
        float shininess = 32.0f;
        glm::vec3 cameraPosWorld = activeCamera->position();

        glUniform3fv(diffuseComponentID, 1, glm::value_ptr(diffuseComponent));
        glUniform3fv(specularComponentID, 1, glm::value_ptr(specularComponent));
        glUniform3fv(ambientComponentID, 1, glm::value_ptr(ambientComponent));
        glUniform1f(shininessID, shininess);
        glUniform3fv(cameraPosWorldID, 1, glm::value_ptr(cameraPosWorld));

        glBindVertexArray(m_VAO);
        glDrawArrays(GL_TRIANGLES, 0, 60);
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

        // Model rotation: J = CCW around Z, K = CW around Z
        float rotateRate = 1.0f;  // radians per second
        if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) {
            modelRotation += rotateRate * (float)timeDiff;
        }
        if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) {
            modelRotation -= rotateRate * (float)timeDiff;
        }

        // Model rotation around X axis: I = CCW, O = CW
        if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) {
            modelRotationX += rotateRate * (float)timeDiff;
        }
        if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) {
            modelRotationX -= rotateRate * (float)timeDiff;
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
