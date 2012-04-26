# $Id$

cmake_minimum_required(VERSION 2.6)
project(num_util)

include(CTest)

find_package(Boost REQUIRED python)
find_package(PythonLibs REQUIRED)

# Build num_util library. Always use -fPIC to enable linking with shared libs.
include_directories(${PYTHON_INCLUDE_DIRS})
add_library(num_util num_util.cpp)
set_target_properties(num_util PROPERTIES COMPILE_FLAGS "-fPIC")
target_link_libraries(num_util ${Boost_LIBRARIES} ${PYTHON_LIBRARIES})
install(TARGETS num_util DESTINATION lib)

if(BUILD_TESTING)
  find_package(PythonInterp REQUIRED)
  # Test program num_py_test
  add_library(num_test_ext MODULE num_test_ext.cpp num_test.cpp)
  set_target_properties(num_test_ext PROPERTIES PREFIX "")
  target_link_libraries(num_test_ext num_util)
  add_test(num_py_test ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/num_py_test.py)
  # Test program simpletest
  add_library(simple_ext MODULE simpletest.cpp)
  set_target_properties(simple_ext PROPERTIES PREFIX "")
  target_link_libraries(simple_ext num_util)
  add_test(simpletest ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/simpletest.py)
endif(BUILD_TESTING)
