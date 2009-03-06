## ------------------------------------------------------------------------------
## Project characteristics

project (CNProc)

cmake_minimum_required (VERSION 2.6)

## ------------------------------------------------------------------------------
## Enable test?

option (CNPROC_ENABLE_TESTING "Enable building and executing test programs?" NO  )

## ------------------------------------------------------------------------------
## Directories to be included in the build

include_directories (${CNProc_BINARY_DIR}/include)

configure_file(${CMAKE_SOURCE_DIR}/config.h.cmake
               ${CNProc_BINARY_DIR}/include/config.h)
configure_file(${CNProc_SOURCE_DIR}/lofar_config.h.cmake
               ${CNProc_BINARY_DIR}/include/lofar_config.h)
	       
add_subdirectory (src)

if (CNPROC_ENABLE_TESTING)
  add_subdirectory (test)
endif (CNPROC_ENABLE_TESTING)  
