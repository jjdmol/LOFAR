project(Tools)

cmake_minimum_required(VERSION 2.6)

configure_file(${CMAKE_SOURCE_DIR}/config.h.cmake
               ${CMAKE_BINARY_DIR}/config.h)
configure_file(${CMAKE_SOURCE_DIR}/lofar_config.h.cmake
               ${CMAKE_BINARY_DIR}/lofar_config.h)

add_subdirectory(src)
