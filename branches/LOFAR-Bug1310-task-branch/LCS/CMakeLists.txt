
## ---------------------------------------------------------------------------
## Project characteristics

project (LCS)

cmake_minimum_required (VERSION 2.6)

enable_testing()

configure_file(${CMAKE_SOURCE_DIR}/config.h.cmake
               ${CMAKE_BINARY_DIR}/config.h)
configure_file(${CMAKE_SOURCE_DIR}/lofar_config.h.cmake
               ${CMAKE_BINARY_DIR}/lofar_config.h)

include(FindBoost)
if(Boost_FOUND)
  add_definitions(-DHAVE_BOOST)
endif(Boost_FOUND)

add_subdirectory(Tools)
add_subdirectory(Common)
