## ------------------------------------------------------------------------------
## Project characteristics

project (Stream)

cmake_minimum_required (VERSION 2.5)

## ------------------------------------------------------------------------------
## Directories to be included in the build

include_directories (${Stream_BINARY_DIR}/include)

configure_file(${CMAKE_SOURCE_DIR}/config.h.cmake
               ${Stream_BINARY_DIR}/include/config.h)
configure_file(${Stream_SOURCE_DIR}/lofar_config.h.cmake
               ${Stream_BINARY_DIR}/include/lofar_config.h)
	       
add_subdirectory (include)
add_subdirectory (src)
## GELSM add_subdirectory (test)
