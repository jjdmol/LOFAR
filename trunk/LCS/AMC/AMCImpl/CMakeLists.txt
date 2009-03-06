
## ------------------------------------------------------------------------------
## Project characteristics

project (AMCImpl)

cmake_minimum_required (VERSION 2.6)

## ------------------------------------------------------------------------------
## Enable test?

option (AMCIMPLE_ENABLE_TESTING "Enable building and executing test programs?" NO  )

## ------------------------------------------------------------------------------
## Directories to be included in the build

include_directories (${AMCImpl_BINARY_DIR}/include)

configure_file(${CMAKE_SOURCE_DIR}/config.h.cmake
               ${AMCImpl_BINARY_DIR}/include/config.h)
configure_file(${AMCImpl_SOURCE_DIR}/lofar_config.h.cmake
               ${AMCImpl_BINARY_DIR}/include/lofar_config.h)
	       
add_subdirectory (include)
add_subdirectory (src)

if (AMCIMPLE_ENABLE_TESTING)
  add_subdirectory (test)
endif (AMCIMPLE_ENABLE_TESTING) 
