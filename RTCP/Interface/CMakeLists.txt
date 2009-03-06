## ------------------------------------------------------------------------------
## Project characteristics

project (Interface)

cmake_minimum_required (VERSION 2.6)

## ------------------------------------------------------------------------------
## Enable test?

option (INTERFACE_ENABLE_TESTING "Enable building and executing test programs?" NO  )

## ------------------------------------------------------------------------------
## Directories to be included in the build

include_directories (${Interface_BINARY_DIR}/include)

configure_file(${CMAKE_SOURCE_DIR}/config.h.cmake
               ${Interface_BINARY_DIR}/include/config.h)
configure_file(${Interface_SOURCE_DIR}/lofar_config.h.cmake
               ${Interface_BINARY_DIR}/include/lofar_config.h)
	       
add_subdirectory (include)
add_subdirectory (src)

if (INTERFACE_ENABLE_TESTING)
  add_subdirectory (test)
endif (INTERFACE_ENABLE_TESTING)  
