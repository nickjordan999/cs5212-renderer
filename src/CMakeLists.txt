add_library (cs4212-util
  ArgumentParsing.cpp ArgumentParsing.h
  FrameBuffer.cpp FrameBuffer.h
  handleGraphicsArgs.cpp handleGraphicsArgs.h
  model_obj.cpp model_obj.h
  vec.h
)
target_compile_definitions(cs4212-util PUBLIC HAS_GLM)
target_link_libraries(cs4212-util PRIVATE Boost::program_options)
target_link_libraries(cs4212-util PUBLIC glm::glm)
target_link_libraries(cs4212-util PUBLIC PNG::PNG)

