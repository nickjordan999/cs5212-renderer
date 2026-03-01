add_library (cs4212-util
  Camera.cpp
  FrameBuffer.cpp FrameBuffer.h
  Shader.cpp Shader.h
  vec.h
)
target_compile_definitions(cs4212-util PUBLIC HAS_GLM)
target_link_libraries(cs4212-util PRIVATE Boost::program_options)
target_link_libraries(cs4212-util PUBLIC glm::glm)
target_link_libraries(cs4212-util PUBLIC PNG::PNG)

# Raytracer tool
add_executable(raytracer raytracer.cpp)
target_link_libraries(raytracer cs4212-util)
