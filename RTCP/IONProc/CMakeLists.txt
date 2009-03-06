## ------------------------------------------------------------------------------
## Project characteristics

project (IONProc)

cmake_minimum_required (VERSION 2.6)

## ------------------------------------------------------------------------------
## Enable test?

option (IONPROC_ENABLE_TESTING "Enable building and executing test programs?" NO  )

## ------------------------------------------------------------------------------
## Directories to be included in the build

include_directories (${IONProc_BINARY_DIR}/include)

configure_file(${CMAKE_SOURCE_DIR}/config.h.cmake
               ${IONProc_BINARY_DIR}/include/config.h)
configure_file(${IONProc_SOURCE_DIR}/lofar_config.h.cmake
               ${IONProc_BINARY_DIR}/include/lofar_config.h)
	       
add_subdirectory (src)

if (IONPROC_ENABLE_TESTING)
  add_subdirectory (test)
endif (IONPROC_ENABLE_TESTING)  
