## ------------------------------------------------------------------------------
## Project characteristics

project (ApplCommon)

cmake_minimum_required (VERSION 2.6)

## ------------------------------------------------------------------------------
## Enable test?

option (APPLCOMMON_ENABLE_TESTING "Enable building and executing test programs?" NO  )

## ------------------------------------------------------------------------------
## Directories to be included in the build

## ------------------------------------------------------------------------------
## Directories to be included in the build

include_directories (${ApplCommon_BINARY_DIR}/include)

configure_file(${CMAKE_SOURCE_DIR}/config.h.cmake
               ${ApplCommon_BINARY_DIR}/include/config.h)
configure_file(${ApplCommon_SOURCE_DIR}/lofar_config.h.cmake
               ${ApplCommon_BINARY_DIR}/include/lofar_config.h)
	       
add_subdirectory (include)
add_subdirectory (src)

if (APPLCOMMON_ENABLE_TESTING)
  add_subdirectory (test)
endif (APPLCOMMON_ENABLE_TESTING)  
