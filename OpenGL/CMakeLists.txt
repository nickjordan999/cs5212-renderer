include_directories(${CMAKE_SOURCE_DIR}/src)

# ---------------------------------------------------------------------------
# glfwExample — original sandbox (kept for reference/learning).
# ---------------------------------------------------------------------------
add_executable(glfwExample
  glfwExample.cpp
  GLSL.cpp GLSL.h
)

target_link_libraries(glfwExample PRIVATE GLEW::GLEW)
target_link_libraries(glfwExample PRIVATE glfw)

if(APPLE)
    target_link_libraries(glfwExample PRIVATE
        "-framework OpenGL"
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreVideo"
    )
else()
    target_link_libraries(glfwExample PRIVATE OpenGL::GL)
endif()

target_link_libraries(glfwExample PRIVATE PNG::PNG)
target_link_libraries(glfwExample PRIVATE ZLIB::ZLIB)

# ---------------------------------------------------------------------------
# openglRenderer — renders raytracer scene presets via OpenGL.
# ---------------------------------------------------------------------------
add_executable(openglRenderer
  openglRenderer.cpp
  GLSL.cpp GLSL.h
  GLMesh.cpp GLMesh.h
  IcoSphere.cpp IcoSphere.h
  SceneBridge.cpp SceneBridge.h
  GLSceneRenderer.cpp GLSceneRenderer.h
)

target_link_libraries(openglRenderer PRIVATE cs4212-util)
target_link_libraries(openglRenderer PRIVATE GLEW::GLEW)
target_link_libraries(openglRenderer PRIVATE glfw)
target_link_libraries(openglRenderer PRIVATE Boost::program_options)

if(APPLE)
    target_link_libraries(openglRenderer PRIVATE
        "-framework OpenGL"
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreVideo"
    )
else()
    target_link_libraries(openglRenderer PRIVATE OpenGL::GL)
endif()

target_link_libraries(openglRenderer PRIVATE PNG::PNG)
target_link_libraries(openglRenderer PRIVATE ZLIB::ZLIB)

# Copy GLSL shader files next to the binary so GLSLObject (which loads by
# bare filename relative to CWD) can find them. This mirrors how glfwExample
# was being run after manually copying shaders into the build dir.
add_custom_command(TARGET openglRenderer POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          ${CMAKE_CURRENT_SOURCE_DIR}/shaders
          $<TARGET_FILE_DIR:openglRenderer>
  COMMENT "Copying GLSL shaders to ${CMAKE_CURRENT_BINARY_DIR}/"
)

# Same for glfwExample so it can find the shaders without manual copying.
add_custom_command(TARGET glfwExample POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          ${CMAKE_CURRENT_SOURCE_DIR}/shaders
          $<TARGET_FILE_DIR:glfwExample>
  COMMENT "Copying GLSL shaders to ${CMAKE_CURRENT_BINARY_DIR}/"
)
