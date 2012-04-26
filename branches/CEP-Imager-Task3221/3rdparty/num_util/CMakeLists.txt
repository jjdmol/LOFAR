# $Id$

cmake_minimum_required(VERSION 2.6)
project(num_util)

include(CTest)

find_package(Boost REQUIRED python)
find_package(PythonLibs REQUIRED)
find_package(PythonInterp REQUIRED)

# Determine the directory that contains the numpy *.h header files.
execute_process(
  COMMAND "${PYTHON_EXECUTABLE}" "-c" 
  "import numpy; print(numpy.get_include())"
  OUTPUT_VARIABLE NUMPY_INCLUDE_DIRS
  ERROR_VARIABLE NUMPY_ERROR
  OUTPUT_STRIP_TRAILING_WHITESPACE)
if(NUMPY_ERROR)
  message(FATAL_ERROR 
    "Failed to determine numpy include directory:\n${NUMPY_ERROR}")
endif(NUMPY_ERROR)

# Set include directories.
include_directories(
  ${Boost_INCLUDE_DIRS} 
  ${PYTHON_INCLUDE_DIRS} 
  ${NUMPY_INCLUDE_DIRS})

# Build num_util library. Always use -fPIC to enable linking with shared libs.
add_library(num_util num_util.cpp)
set_target_properties(num_util PROPERTIES COMPILE_FLAGS "-fPIC")
target_link_libraries(num_util ${Boost_LIBRARIES} ${PYTHON_LIBRARIES})
install(TARGETS num_util DESTINATION lib)

if(BUILD_TESTING)
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
