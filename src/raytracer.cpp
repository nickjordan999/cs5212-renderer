#include "FrameBuffer.h"
#include "vec.h"
#include "Ray.h"
#include "Camera.h"
#include "Scene.h"
#include "ScenePresets.h"
#include "Shader.h"
#include "Sphere.h"
#include "Light.h"
#include <iostream>
#include <fstream>
#include <cstdio>
#include <cmath>
#include <unistd.h>
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

void printUsage(const std::string &programName, const po::options_description &desc)
{
  std::cerr << "Usage: " << programName << " [OPTIONS]" << std::endl;
  std::cerr << std::endl;
  std::cerr << "Available Scene Presets:" << std::endl;
  std::cerr << "  test                 - Standard test scene with colored spheres (default)" << std::endl;
  std::cerr << "  lit_test             - Test scene with lighting for diffuse rendering" << std::endl;
  std::cerr << "  single_sphere        - Single purple sphere" << std::endl;
  std::cerr << "  grid                 - 3x3 grid of spheres" << std::endl;
  std::cerr << "  aligned              - 50 spheres lined up on z-axis with random colors" << std::endl;
  std::cerr << "  spiral               - 50 spheres spiraling around z-axis with random colors" << std::endl;
  std::cerr << "  pyramid              - Triangle pyramid with colored faces" << std::endl;
  std::cerr << "  checkerboard         - Checkerboard plane with mirror sphere" << std::endl;
  std::cerr << "  hexboard             - Hexagonal-tiled plane with mirror sphere" << std::endl;
  std::cerr << "  platonic             - All five Platonic solids on a checkerboard" << std::endl;
  std::cerr << "  dodecahedron         - Floating Dodecahedron" << std::endl;
  std::cerr << "  mixed_shader         - Three spheres each locked to a different shader (demo)" << std::endl;
  std::cerr << "  shadow_demo          - Single sphere on a checkerboard plane with three lights casting shadows" << std::endl;
  std::cerr << "  trilist              - Load triangles from a data file (requires --datafile)" << std::endl;
  std::cerr << std::endl;
  std::cerr << "Available Shaders:" << std::endl;
  std::cerr << "  render               - Renders scene with material colors (default)" << std::endl;
  std::cerr << "  normalshader         - Renders scene with normal visualization" << std::endl;
  std::cerr << "  lambertian           - Renders scene with Lambertian (diffuse) shading" << std::endl;
  std::cerr << "  blinnphong           - Renders scene with Blinn-Phong shading" << std::endl;
  std::cerr << "  mirror               - Renders every surface as a reflective mirror (use --reflect-depth to control bounces)" << std::endl;
  std::cerr << "  diffuse              - Renders scene with path-traced diffuse shading (indirect lighting)" << std::endl;
  std::cerr << std::endl;
  std::cerr << "Rendering Options:" << std::endl;
  std::cerr << "  --shadows on/off     - Enable or disable shadow casting (default: on)" << std::endl;
  std::cerr << "  --threads <int>      - Number of rendering threads (default: CPU core count)" << std::endl;
  std::cerr << std::endl;
  std::cerr << "Options:" << std::endl;
  std::cerr << desc << std::endl;
  std::cerr << "Examples:" << std::endl;
  std::cerr << "  " << programName << " --width 1024 --height 768" << std::endl;
  std::cerr << "  " << programName << " -w 800 -h 600 -s diffuse -p grid --rays-per-pixel 16" << std::endl;
  std::cerr << "  " << programName << " -p spiral --scene-param t=0.5" << std::endl;
  std::cerr << "  " << programName << " --anti-aliasing off" << std::endl;
  std::cerr << "  " << programName << " -p trilist --datafile data/trilist.dat -s diffuse" << std::endl;
}

// Load a scene by preset name
Scene loadScene(const std::string &preset_name, const SceneParams &params, const std::string &datafile = "")
{
  if (preset_name == "trilist") {
    if (datafile.empty()) {
      throw std::invalid_argument("The 'trilist' preset requires --datafile <path>");
    }
    return ScenePresets::createTrilistScene(datafile);
  } else if (preset_name == "test") {
    return ScenePresets::createTestScene();
  } else if (preset_name == "lit_test") {
    return ScenePresets::createLitTestScene();
  } else if (preset_name == "single_sphere") {
    return ScenePresets::createSingleSphereScene();
  } else if (preset_name == "grid") {
    return ScenePresets::createGridScene();
  } else if (preset_name == "aligned") {
    return ScenePresets::createAlignedSpheresScene();
  } else if (preset_name == "spiral") {
    return ScenePresets::createSpiralSpheresScene(params);
  } else if (preset_name == "pyramid") {
    return ScenePresets::createTrianglePyramidScene();
  } else if (preset_name == "octahedron") {
    return ScenePresets::createOctahedronScene();
  } else if (preset_name == "dodecahedron") {
    return ScenePresets::createDodecahedronScene();
  } else if (preset_name == "checkerboard") {
    return ScenePresets::createCheckerboardScene();
  } else if (preset_name == "hexboard") {
    return ScenePresets::createHexboardScene();
  } else if (preset_name == "platonic") {
    return ScenePresets::createPlatonicSolidsScene();
  } else if (preset_name == "triangle_test") {
    return ScenePresets::createTriangleTestScene();
  } else if (preset_name == "mixed_shader") {
    return ScenePresets::createMixedShaderScene(params);
  } else if (preset_name == "shadow_demo") {
    return ScenePresets::createShadowDemoScene();
  } else if (preset_name == "hall_of_mirrors") {
    return ScenePresets::createHallOfMirrorsScene();
  } else if (preset_name == "random_spheres") {
    return ScenePresets::createRandomSpheresScene(params);
  } else {
    std::cerr << "Unknown scene preset: " << preset_name << std::endl;
    throw std::invalid_argument("Invalid scene preset");
  }
}

// Create a shader based on the rendering mode
std::unique_ptr<Shader> createShader(const std::string &mode, const Scene &scene, int reflectDepth, bool shadows, int numThreads)
{
  if (mode == "render") {
    return std::make_unique<SimpleShader>(scene, numThreads);
  } else if (mode == "normalshader") {
    return std::make_unique<NormalShader>(scene, numThreads);
  } else if (mode == "lambertian") {
    return std::make_unique<LambertianShader>(scene, shadows, numThreads);
  } else if (mode == "blinnphong") {
    return std::make_unique<BlinnPhongShader>(scene, shadows, numThreads);
  } else if (mode == "mirror") {
    return std::make_unique<MirrorShader>(scene, reflectDepth, numThreads);
  } else if (mode == "diffuse") {
    return std::make_unique<DiffuseShader>(scene, shadows, reflectDepth, numThreads);
  } else {
    throw std::invalid_argument("Unknown shader mode: " + mode);
  }
}

int main(int argc, char *argv[])
{
  try {
    // Define command-line options
    po::options_description desc("Allowed options");
    desc.add_options()("help", "Show this help message")("anti-aliasing,a", po::value<std::string>()->default_value("on"), "Toggle anti-aliasing (on/off)")("rays-per-pixel", po::value<int>()->default_value(8), "Number of rays per pixel for anti-aliasing")("width,w", po::value<int>()->default_value(800), "Image width in pixels")("height,h", po::value<int>()->default_value(800), "Image height in pixels")("focal-length,l", po::value<float>()->default_value(1.0f), "Focal length to image plane")("scene-preset,p", po::value<std::string>()->default_value("test"), "Scene preset to use")("shader,s", po::value<std::string>()->default_value("render"), "Shader mode to use")("reflect-depth", po::value<int>()->default_value(5), "Maximum mirror reflection bounces (used by --shader mirror)")("shadows", po::value<std::string>()->default_value("on"), "Enable shadow casting (on/off)")("threads", po::value<int>()->default_value(0), "Number of rendering threads (0 = auto-detect)")("scene-param", po::value<std::vector<std::string>>()->composing(), "Scene parameter as key=value (e.g. t=0.5)")("datafile,d", po::value<std::string>()->default_value(""), "Path to triangle data file (used with -p trilist)");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    // Handle help option
    if (vm.count("help")) {
      printUsage(argv[0], desc);
      return 0;
    }

    // Parse options
    int width = vm["width"].as<int>();
    int height = vm["height"].as<int>();
    float focal_length = vm["focal-length"].as<float>();
    std::string scene_preset = vm["scene-preset"].as<std::string>();
    std::string shader_mode = vm["shader"].as<std::string>();
    std::string aa_mode = vm["anti-aliasing"].as<std::string>();
    int rays_per_pixel = vm["rays-per-pixel"].as<int>();
    int reflect_depth = vm["reflect-depth"].as<int>();
    std::string shadows_mode = vm["shadows"].as<std::string>();
    bool shadows_enabled = (shadows_mode == "on" || shadows_mode == "true" || shadows_mode == "1");
    int num_threads = vm["threads"].as<int>();
    std::string datafile = vm["datafile"].as<std::string>();

    // Parse anti-aliasing mode
    bool anti_aliasing_enabled = false;
    if (aa_mode == "on" || aa_mode == "true" || aa_mode == "1") {
      anti_aliasing_enabled = true;
    } else if (aa_mode == "off" || aa_mode == "false" || aa_mode == "0") {
      anti_aliasing_enabled = false;
    } else {
      std::cerr << "Error: --anti-aliasing must be 'on' or 'off'" << std::endl;
      return 1;
    }

    // Validate parameters
    if (width <= 0 || height <= 0) {
      std::cerr << "Error: width and height must be positive integers" << std::endl;
      return 1;
    }

    if (rays_per_pixel <= 0) {
      std::cerr << "Error: rays-per-pixel must be a positive integer" << std::endl;
      return 1;
    }

    if (focal_length <= 0) {
      std::cerr << "Error: focal-length must be positive" << std::endl;
      return 1;
    }

    // Parse scene parameters (key=value pairs)
    SceneParams scene_params;
    if (vm.count("scene-param")) {
      for (const auto &kv : vm["scene-param"].as<std::vector<std::string>>()) {
        size_t eq = kv.find('=');
        if (eq == std::string::npos) {
          std::cerr << "Error: --scene-param must be key=value, got: " << kv << std::endl;
          return 1;
        }
        scene_params[kv.substr(0, eq)] = std::stof(kv.substr(eq + 1));
      }
    }

    // Set actual rays per pixel based on anti-aliasing setting
    int actual_rays_per_pixel = anti_aliasing_enabled ? rays_per_pixel : 1;

    // Load scene (for diffuse rendering, use lit version if available)
    Scene scene;
    scene = loadScene(scene_preset, scene_params, datafile);

    // Update camera resolution to match requested dimensions and focal length
    // auto camera_ptr = std::make_shared<PerspectiveBasicCamera>(
    //     vec3(0.0f, 0.0f, 3.0f), vec3(0.0f, 0.0f, -1.0f), focal_length,
    //     static_cast<float>(width), static_cast<float>(height)
    // );
    // scene.setCamera(camera_ptr);

    // Create appropriate shader
    auto shader = createShader(shader_mode, scene, reflect_depth, shadows_enabled, num_threads);

    // Create framebuffer and render
    FrameBuffer fb(width, height);
    shader->traceScene(fb, actual_rays_per_pixel);

    // Write to stdout
    char tmpfile[] = "/tmp/raytracer_XXXXXX";
    int tmpfd = mkstemp(tmpfile);
    if (tmpfd < 0) {
      std::cerr << "Error creating temporary file" << std::endl;
      return 1;
    }
    close(tmpfd);

    try {
      fb.writeToPng(tmpfile);

      std::ifstream infile(tmpfile, std::ios::binary);
      if (!infile) {
        std::cerr << "Error reading temporary PNG file" << std::endl;
        std::remove(tmpfile);
        return 1;
      }

      std::cout << infile.rdbuf();
      infile.close();

      std::remove(tmpfile);
    } catch (const std::exception &e) {
      std::remove(tmpfile);
      throw;
    }

    return 0;
  } catch (const po::error &e) {
    std::cerr << "Error parsing command-line arguments: " << e.what() << std::endl;
    return 1;
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
}
