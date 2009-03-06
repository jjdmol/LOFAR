## ------------------------------------------------------------------------------
## Project characteristics

project (Transport)

cmake_minimum_required (VERSION 2.6)

## ------------------------------------------------------------------------------
## Enable test?

option (TRANSPORT_ENABLE_TESTING "Enable building and executing test programs?" NO  )

## ------------------------------------------------------------------------------
## Directories to be included in the build

include_directories (${Transport_BINARY_DIR}/include)

configure_file(${CMAKE_SOURCE_DIR}/config.h.cmake
               ${Transport_BINARY_DIR}/include/config.h)
configure_file(${Transport_SOURCE_DIR}/lofar_config.h.cmake
               ${Transport_BINARY_DIR}/include/lofar_config.h)
	       
add_subdirectory (include)
add_subdirectory (src)

if (TRANSPORT_ENABLE_TESTING)
  add_subdirectory (test)
endif (TRANSPORT_ENABLE_TESTING)  
