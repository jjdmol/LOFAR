project(Common)

cmake_minimum_required(VERSION 2.6)

include(CheckTypeSize)
check_type_size("ushort" HAVE_USHORT)
check_type_size("uint" HAVE_UINT)
check_type_size("ulong" HAVE_ULONG)
check_type_size("long long" HAVE_LONG_LONG)

message(STATUS "HAVE_USHORT=${HAVE_USHORT}")
message(STATUS "HAVE_UINT=${HAVE_UINT}")
message(STATUS "HAVE_ULONG=${HAVE_ULONG}")
message(STATUS "HAVE_LONG_LONG=${HAVE_LONG_LONG}")

add_definitions(-DHAVE_CONFIG_H)
add_definitions(-DHAVE_BOOST)

add_subdirectory(include/Common)
add_subdirectory(src)
include(CTest)
if(BUILD_TESTING)
  add_subdirectory(test)
endif(BUILD_TESTING)

message(STATUS "CMAKE_SOURCE_DIR=${CMAKE_SOURCE_DIR}")
message(STATUS "CMAKE_BINARY_DIR=${CMAKE_BINARY_DIR}")
message(STATUS "CMAKE_CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "CMAKE_CURRENT_BINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}")

#set(CTEST_ENVIRONMENT
#  "lofar_sharedir=aap/noot/mies")

configure_file(${CMAKE_SOURCE_DIR}/config.h.cmake
               ${CMAKE_BINARY_DIR}/config.h)
configure_file(${CMAKE_SOURCE_DIR}/lofar_config.h.cmake
               ${CMAKE_BINARY_DIR}/lofar_config.h)
