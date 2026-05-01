// OpenGL renderer for raytracer scene presets.
//
// Mirrors the relevant subset of raytracer.cpp options (--scene-preset,
// --scene-param, --shader, --width/--height, --datafile). Opens a GLFW window
// and renders the chosen Scene with Lambertian / Blinn-Phong / Normal / Simple
// shading. WASD/J/K/I/O/N/M control the camera and model transform.

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <boost/program_options.hpp>

#include "ScenePresets.h"
#include "Scene.h"

#include "Camera.h"           // OpenGL/Camera.h: GLCamera + GLPerspectiveCamera
#include "GLSceneRenderer.h"
#include "SceneBridge.h"

namespace po = boost::program_options;

namespace {

void printUsage(const std::string &programName, const po::options_description &desc)
{
  std::cerr << "Usage: " << programName << " [OPTIONS]\n\n"
            << "Renders a raytracer scene preset using OpenGL.\n\n"
            << "Available scene presets (same names as raytracer):\n"
            << "  test, lit_test, single_sphere, grid, aligned, spiral,\n"
            << "  pyramid, octahedron, dodecahedron, checkerboard, hexboard,\n"
            << "  platonic, triangle_test, mixed_shader, shadow_demo,\n"
            << "  hall_of_mirrors, random_spheres, fresnel_caustic, trilist\n\n"
            << "Available shaders (default: blinnphong):\n"
            << "  render        - flat material color (no lighting)\n"
            << "  normalshader  - normal-as-color\n"
            << "  lambertian    - diffuse only\n"
            << "  blinnphong    - diffuse + specular + ambient\n\n"
            << "Note: per-material overrides in the scene presets still take effect.\n"
            << "Materials with MIRROR / DIELECTRIC / PATH_DIFFUSE fall back to Blinn-Phong.\n\n"
            << "Controls (fly camera):\n"
            << "  W / S          - fly forward / backward\n"
            << "  A / D          - strafe left / right\n"
            << "  Space / LShift - fly up / down (world-up)\n"
            << "  Arrows         - look around (yaw / pitch)\n"
            << "  ESC            - quit\n\n"
            << desc << std::endl;
}

gl_bridge::GLShaderKind shaderKindFromFlag(const std::string &mode)
{
  using K = gl_bridge::GLShaderKind;
  if (mode == "render")        return K::Simple;
  if (mode == "normalshader")  return K::Normal;
  if (mode == "lambertian")    return K::Lambertian;
  if (mode == "blinnphong")    return K::BlinnPhong;
  if (mode == "mirror" || mode == "diffuse") {
    std::cerr << "[openglRenderer] '" << mode
              << "' is raytracer-only; falling back to blinnphong.\n";
    return K::BlinnPhong;
  }
  std::cerr << "[openglRenderer] unknown shader '" << mode
            << "', falling back to blinnphong.\n";
  return K::BlinnPhong;
}

}  // namespace

int main(int argc, char *argv[])
{
  try {
    po::options_description desc("Allowed options");
    desc.add_options()
      ("help,?",      "Show this help message")
      ("width,w",     po::value<int>()->default_value(800), "Window width in pixels")
      ("height,h",    po::value<int>()->default_value(800), "Window height in pixels")
      ("scene-preset,p", po::value<std::string>()->default_value("test"), "Scene preset name")
      ("shader,s",    po::value<std::string>()->default_value("blinnphong"),
                      "Default shader: render, normalshader, lambertian, blinnphong")
      ("scene-param", po::value<std::vector<std::string>>()->composing(),
                      "Scene parameter as key=value (e.g. t=0.5)")
      ("datafile,d",  po::value<std::string>()->default_value(""),
                      "Path to triangle data file (used with -p trilist)");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
      printUsage(argv[0], desc);
      return 0;
    }

    int winWidth  = vm["width"].as<int>();
    int winHeight = vm["height"].as<int>();
    std::string preset = vm["scene-preset"].as<std::string>();
    std::string shaderMode = vm["shader"].as<std::string>();
    std::string datafile = vm["datafile"].as<std::string>();

    if (winWidth <= 0 || winHeight <= 0) {
      std::cerr << "Error: width/height must be positive\n";
      return 1;
    }

    SceneParams params;
    if (vm.count("scene-param")) {
      for (const auto &kv : vm["scene-param"].as<std::vector<std::string>>()) {
        size_t eq = kv.find('=');
        if (eq == std::string::npos) {
          std::cerr << "Error: --scene-param must be key=value, got: " << kv << "\n";
          return 1;
        }
        params[kv.substr(0, eq)] = std::stof(kv.substr(eq + 1));
      }
    }

    Scene scene = ScenePresets::loadScene(preset, params, datafile);

    gl_bridge::GLShaderKind defaultKind = shaderKindFromFlag(shaderMode);

    // ---------- GLFW + GLEW init ----------
    if (!glfwInit()) {
      std::cerr << "Error: glfwInit failed\n";
      return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow *window = glfwCreateWindow(winWidth, winHeight,
                                          ("OpenGL Renderer — " + preset).c_str(),
                                          nullptr, nullptr);
    if (!window) {
      std::cerr << "Error: glfwCreateWindow failed\n";
      glfwTerminate();
      return 1;
    }
    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
      std::cerr << "Error: glewInit failed\n";
      return 1;
    }

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    glViewport(0, 0, fbWidth, fbHeight);
    float aspect = static_cast<float>(fbWidth) / static_cast<float>(fbHeight);

    // ---------- Renderer init ----------
    gl_bridge::GLSceneRenderer renderer;
    renderer.init(scene, defaultKind, fbWidth, fbHeight);

    // ---------- Camera (interactive, seeded from the preset) ----------
    std::unique_ptr<GLPerspectiveCamera> camera;
    if (scene.hasCamera()) {
      camera = gl_bridge::bridgeCamera(scene.getCamera(), aspect);
    } else {
      camera = std::make_unique<GLPerspectiveCamera>(
          glm::vec3(0, 0, 5), glm::vec3(0, 0, -1), glm::vec3(0, 1, 0),
          glm::radians(45.0f), aspect, 0.1f, 1000.0f);
    }

    // Keep the GL viewport and camera aspect in sync with the window size.
    glfwSetWindowUserPointer(window, camera.get());
    glfwSetFramebufferSizeCallback(window,
        [](GLFWwindow *win, int w, int h) {
          if (w <= 0 || h <= 0) return;
          glViewport(0, 0, w, h);
          auto *cam = static_cast<GLPerspectiveCamera *>(glfwGetWindowUserPointer(win));
          if (cam) cam->setAspectRatio(static_cast<float>(w) / static_cast<float>(h));
        });

    // ---------- Frame loop ----------
    // Yaw/pitch derived from the seeded camera's forward, then accumulated from
    // arrow-key input. Pitch is clamped just under ±90° to prevent gimbal flip.
    const glm::vec3 worldUp(0.0f, 1.0f, 0.0f);
    glm::vec3 initFwd = glm::normalize(camera->forward());
    float yaw   = std::atan2(initFwd.z, initFwd.x);
    float pitch = std::asin(glm::clamp(initFwd.y, -1.0f, 1.0f));
    const float maxPitch = glm::radians(89.0f);

    double prevTime = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
      double now = glfwGetTime();
      double dt  = now - prevTime;
      prevTime   = now;
      float dts  = static_cast<float>(dt);

      renderer.render(*camera);
      glfwSwapBuffers(window);
      glfwPollEvents();

      // ---- Look (arrow keys: yaw/pitch) ----
      const float yawRate   = glm::radians(90.0f);   // rad/sec
      const float pitchRate = glm::radians(60.0f);
      if (glfwGetKey(window, GLFW_KEY_LEFT)  == GLFW_PRESS) yaw   -= yawRate   * dts;
      if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) yaw   += yawRate   * dts;
      if (glfwGetKey(window, GLFW_KEY_UP)    == GLFW_PRESS) pitch += pitchRate * dts;
      if (glfwGetKey(window, GLFW_KEY_DOWN)  == GLFW_PRESS) pitch -= pitchRate * dts;
      pitch = glm::clamp(pitch, -maxPitch, maxPitch);

      glm::vec3 forward = glm::normalize(glm::vec3(
        std::cos(pitch) * std::cos(yaw),
        std::sin(pitch),
        std::cos(pitch) * std::sin(yaw)));
      camera->setForward(forward);

      // ---- Move (WASD camera-relative; Space/LShift world-relative up/down) ----
      const float moveRate = 5.0f;  // world units per second
      glm::vec3 camPos = camera->position();
      glm::vec3 camRight = glm::normalize(glm::cross(forward, worldUp));
      float step = moveRate * dts;

      if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camPos += forward  * step;
      if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camPos -= forward  * step;
      if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camPos -= camRight * step;
      if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camPos += camRight * step;
      if (glfwGetKey(window, GLFW_KEY_SPACE)      == GLFW_PRESS) camPos += worldUp * step;
      if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) camPos -= worldUp * step;
      camera->setPosition(camPos);

      if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, 1);
      }
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
  } catch (const po::error &e) {
    std::cerr << "Error parsing command-line arguments: " << e.what() << "\n";
    return 1;
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
  }
}
