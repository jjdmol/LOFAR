## ------------------------------------------------------------------------------
## Project characteristics

project (Blob)

cmake_minimum_required (VERSION 2.6)

## ------------------------------------------------------------------------------
## Enable test?

option (BLOB_ENABLE_TESTING "Enable building and executing test programs?" NO  )

## ------------------------------------------------------------------------------
## Directories to be included in the build

include_directories (${Blob_BINARY_DIR}/include)

configure_file(${CMAKE_SOURCE_DIR}/config.h.cmake
               ${Blob_BINARY_DIR}/include/config.h)
configure_file(${Blob_SOURCE_DIR}/lofar_config.h.cmake
               ${Blob_BINARY_DIR}/include/lofar_config.h)
	       
add_subdirectory (include)
add_subdirectory (src)

if (BLOB_ENABLE_TESTING)
  add_subdirectory (test)
endif (BLOB_ENABLE_TESTING)  
