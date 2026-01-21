# 
# Adds the src directory to the include search
# path. ${CMAKE_SOURCE_DIR} is prefixed to allow the files in the
# utest directory to correctly include the relative reference to the
# src files.
#
include_directories(${CMAKE_SOURCE_DIR}/src)

enable_testing()
include(CTest)
include(Catch)

# 
# This is a simple way to set the names of the executables used for
# the unit tests. Note that we are setting a variable, called
# ${UTESTS} to be the set of unit test executables. Then, in the next
# step, we iterate over all the elements within the ${UTESTS} set to
# compile into an executable.
# 
set(UTESTS 
  utest_Success
  utest_vec)

# 
# For each of the executables named in ${UTESTS}, compile them into a
# boost unit test for use by the CMake CTest system.
# 
foreach(uexec ${UTESTS})
  add_executable(${uexec} ${uexec}.cpp)
  target_link_libraries(${uexec} PRIVATE Catch2::Catch2WithMain)
  target_link_libraries(${uexec} PRIVATE cs4212-util)

  add_test(NAME ${uexec} COMMAND ${uexec})
endforeach(uexec)

