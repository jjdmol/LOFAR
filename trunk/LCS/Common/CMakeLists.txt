## ------------------------------------------------------------------------------
## Project characteristics

project (Common)

cmake_minimum_required (VERSION 2.5)

## ------------------------------------------------------------------------------
## Enable test?

option (COMMON_ENABLE_TESTING "Enable building and executing test programs?" NO  )

## ------------------------------------------------------------------------------
## Directories to be included in the build

include_directories (${Common_BINARY_DIR}/include)

configure_file(${CMAKE_SOURCE_DIR}/config.h.cmake
               ${Common_BINARY_DIR}/include/config.h)
configure_file(${Common_SOURCE_DIR}/lofar_config.h.cmake
               ${Common_BINARY_DIR}/include/lofar_config.h)
	       
add_subdirectory (include)
add_subdirectory (src)

if (COMMON_ENABLE_TESTING)
  add_subdirectory (test)
endif (COMMON_ENABLE_TESTING)  
