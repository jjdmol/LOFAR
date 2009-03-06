## ------------------------------------------------------------------------------
## Project characteristics

project (Storage)

cmake_minimum_required (VERSION 2.6)

## ------------------------------------------------------------------------------
## Enable test?

option (STORAGE_ENABLE_TESTING "Enable building and executing test programs?" YES  )

## ------------------------------------------------------------------------------
## Directories to be included in the build

include_directories (${Storage_BINARY_DIR}/include)

configure_file(${CMAKE_SOURCE_DIR}/config.h.cmake
               ${Storage_BINARY_DIR}/include/config.h)
configure_file(${Storage_SOURCE_DIR}/lofar_config.h.cmake
               ${Storage_BINARY_DIR}/include/lofar_config.h)
	       
add_subdirectory (include)
add_subdirectory (src)

if (STORAGE_ENABLE_TESTING)
  add_subdirectory (test)
endif (STORAGE_ENABLE_TESTING)  
