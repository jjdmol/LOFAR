execute_process (COMMAND pwd
    OUTPUT_VARIABLE path
    OUTPUT_STRIP_TRAILING_WHITESPACE)

## ----------------------------------------------------------------------------
##  Host-specific overrides

function(check_package_name _NAME result_var)
  if ("${_NAME}" MATCHES "^$")
    set(${result_var} 0 PARENT_SCOPE)
    break()
  elseif (${_NAME} STREQUAL "IONProc")
    set(${result_var} 1 PARENT_SCOPE)
    break()
  elseif (${_NAME} STREQUAL "CNProc") 
    set(${result_var} 1 PARENT_SCOPE)
    break()
  elseif (${_NAME} STREQUAL "Storage") 
    set(${result_var} 1 PARENT_SCOPE)
    break()
  else ("${_NAME}" MATCHES "^$")
    set(${result_var} 0 PARENT_SCOPE)
  endif ("${_NAME}" MATCHES "^$") 
endfunction(check_package_name)  

set (PACKAGE_NAME "${PACKAGE_NAME}")

check_package_name("${PACKAGE_NAME}" valid_package_name)
if (NOT ${valid_package_name})
  message (FATAL_ERROR "Illegal package name: (${PACKAGE_NAME})")
endif(NOT ${valid_package_name})

execute_process (COMMAND hostname -s
  OUTPUT_VARIABLE hostname
  OUTPUT_STRIP_TRAILING_WHITESPACE)

set (LOFAR_VARIANTS_FILE ${path}/../../cmake/variants/variants.${hostname})

message (STATUS "Loading settings variants ${path}/../../cmake/variants/variants")
include (${path}/../../cmake/variants/variants)

if (EXISTS ${LOFAR_VARIANTS_FILE})
  message (STATUS "Loading settings variants " ${LOFAR_VARIANTS_FILE})
  include (${LOFAR_VARIANTS_FILE})
endif (EXISTS ${LOFAR_VARIANTS_FILE})
  
## Get compiler: e.g. ~/projects/LOFAR/build/gnu_openmpi-opt
get_filename_component (BUILD_VARIANT ${path} NAME)


#set ( BUILD_VARIANT gnubgp_cn )
STRING (REGEX MATCH ".*_" compiler ${BUILD_VARIANT})
STRING (REGEX REPLACE "_" "" compiler ${compiler})

## Get variant
STRING (REGEX MATCH "_.*" variant ${BUILD_VARIANT})
STRING (REGEX REPLACE "_" "" variant ${variant})

foreach (comp ${${compiler}_compiler})
  STRING (REGEX MATCH "^C[CX].*=" result ${comp})
  if (${result} STREQUAL "CC=")
    STRING (REGEX MATCH "/.*" cc ${comp})
    set( CMAKE_C_COMPILER ${cc} CACHE INTERNAL "gcc Compiler" FORCE )
  elseif (${result} STREQUAL "CXX=")
    STRING (REGEX MATCH "/.*" cc ${comp})
    set( CMAKE_CXX_COMPILER ${cc} CACHE INTERNAL "g++ Compiler" FORCE )
    if (${variant} STREQUAL "ion") 
      set( CXX_COMPILE_FLAGS "${CXX_COMPILE_FLAGS} ${${variant}}" CACHE STRING "g++ Compiler flags" FORCE )
    else (${variant} STREQUAL "ion") 
      set( CXX_COMPILE_FLAGS "${${variant}}" CACHE STRING "g++ Compiler flags" FORCE )
    endif (${variant} STREQUAL "ion")
  elseif (${result} STREQUAL "CCAS=")
    STRING (REGEX MATCH "/.*" asm ${comp})
    set( CCAS "${asm}" CACHE STRING "Assembler" FORCE )
  else (${result} STREQUAL "CC=")
    message(SEND_ERROR "Invalid compiler: ${comp}")
  endif (${result} STREQUAL "CC=")
endforeach (comp ${${compiler}_compiler})

## ------------------------------------------------------------------------------
## Project characteristics

project (LOFAR)

cmake_minimum_required (VERSION 2.6)

## ------------------------------------------------------------------------------
## Set CMAKE_MODULE_PATH to load custom CMake modules

find_path (LOFAR_ROOT cmake/modules/CMakeSettings.cmake
  PATHS 
  ${LOFAR_SOURCE_DIR}
  ${LOFAR_SOURCE_DIR}/..
  ${LOFAR_SOURCE_DIR}/../..
  ${LOFAR_SOURCE_DIR}/../../..
  $ENV{LOFARSOFT}
  )

if (LOFAR_ROOT)
  include (${LOFAR_ROOT}/cmake/modules/CMakeSettings.cmake)
else (LOFAR_ROOT)
  message (FATAL_ERROR "Unable to locate additional CMake scripts!")
endif (LOFAR_ROOT)

## -----------------------------------------------------------------------------
## Include Lofar general

include (LofarGeneral)

## ------------------------------------------------------------------------------
## Set debug options

option (LOFAR_VERBOSE_CONFIGURE "Verbose output during configuration?" 1)
option (LOFAR_BUILD_TESTS       "Build the test programs?"             1)

## ------------------------------------------------------------------------------
## Additional CMake modules

include (CheckIncludeFiles)
include (CheckLibraryExists)
include (CheckCXXSourceCompiles)
include (CheckTypeSize)
include (CTest)
enable_testing()

## ------------------------------------------------------------------------------
## Configure file

configure_file(${CMAKE_SOURCE_DIR}/config.h.cmake
               ${CMAKE_BINARY_DIR}/config.h)
configure_file(${CMAKE_SOURCE_DIR}/lofar_config.h.cmake
               ${CMAKE_BINARY_DIR}/lofar_config.h)

## ------------------------------------------------------------------------------
## Search for external libraries

if (NOT LOFAR_VERBOSE_CONFIGURE)
  set (BOOST_FIND_QUIETLY TRUE)
  set (CASACORE_FIND_QUIETLY FALSE)
endif (NOT LOFAR_VERBOSE_CONFIGURE)

## Load module(s)
foreach (mod ${${PACKAGE_NAME}_modules})
  include(${CMAKE_MODULE_PATH}/${mod}.cmake)
  STRING (REGEX REPLACE "Find" "" mod ${mod})
  if (${mod}_FOUND)
    string(TOUPPER ${mod} mod)
    if (${mod} STREQUAL "BOOST")
      include_directories (${${mod}_INCLUDE_DIR})
    elseif (${mod} STREQUAL "MPI")
      include_directories (${${mod}_INCLUDE_PATH})
    else (${mod} STREQUAL "BOOST")
      include_directories (${${mod}_INCLUDES})
    endif (${mod} STREQUAL "BOOST")
    if (${mod} STREQUAL "CASACORE")
      add_definitions (-DHAVE_AIPSPP)
    else (${mod} STREQUAL "CASACORE")
      add_definitions (-DHAVE_${mod})
    endif (${mod} STREQUAL "CASACORE")
    if (${mod} STREQUAL "MPI")
      set( CMAKE_CXX_COMPILER ${MPI_COMPILER} CACHE INTERNAL "MPI Compiler" FORCE )
    endif (${mod} STREQUAL "MPI")
  endif (${mod}_FOUND)
endforeach (mod ${${PACKAGE_NAME}_modules})

## ------------------------------------------------------------------------------
## Commonly used configuration files

if (${compiler} STREQUAL "gnu" OR ${compiler} STREQUAL "gnubgp")
  set( CXX_COMPILE_FLAGS "${CXX_COMPILE_FLAGS} -W -Wall -Woverloaded-virtual -Wno-unknown-pragmas" CACHE STRING "CXX Compiler flags" FORCE )
  add_definitions (${CXX_COMPILE_FLAGS})
endif (${compiler} STREQUAL "gnu" OR ${compiler} STREQUAL "gnubgp")

## Common compiler flags

## ------------------------------------------------------------------------------
## Module header files

include_directories (
  ## LCS
  ${LOFAR_SOURCE_DIR}/LCS/Blob/include
  ${LOFAR_SOURCE_DIR}/LCS/Common/include
  ${LOFAR_SOURCE_DIR}/LCS/ACC/APS/include
  ${LOFAR_SOURCE_DIR}/LCS/ACC/PLC/include
  ${LOFAR_SOURCE_DIR}/LCS/AMC/AMCBase/include
  ${LOFAR_SOURCE_DIR}/LCS/AMC/AMCImpl/include
  ${LOFAR_SOURCE_DIR}/LCS/Transport/include
  ${LOFAR_SOURCE_DIR}/LCS/Stream/include
  ${LOFAR_SOURCE_DIR}/LCS/ApplCommon/include
  ## RTCP
  ${LOFAR_SOURCE_DIR}/RTCP/Interface/include
  ${LOFAR_SOURCE_DIR}/RTCP/Storage/include
  ${LOFAR_SOURCE_DIR}/RTCP/IONProc/src
  ${LOFAR_SOURCE_DIR}/RTCP/CNProc/src
  )

## ------------------------------------------------------------------------------
## Check for the presence of various packages in the code tree


## ------------------------------------------------------------------------------
## Directories to be included in the build

## --- LCS ---------------------------------------

find_path (HAVE_LCS CMakeLists.txt ${LOFAR_SOURCE_DIR}/LCS)
if (HAVE_LCS)
  set (HAVE_LCS TRUE CACHE BOOL "Have package LCS?" FORCE)
  add_subdirectory (LCS)
endif (HAVE_LCS)  

### --- RTCP ---------------------------------------

find_path (HAVE_RTCP CMakeLists.txt ${LOFAR_SOURCE_DIR}/RTCP)
if (HAVE_RTCP)
  set (HAVE_RTCP TRUE CACHE BOOL "Have package RTCP?" FORCE)
  add_subdirectory (RTCP)
endif (HAVE_RTCP)  

# ------------------------------------------------------------------------------
## Additional target dependencies 
message (STATUS "TEST_PROGRAMS      = ${TEST_PROGRAMS}")
add_custom_target (check
  ctest
  DEPENDS ${TEST_PROGRAMS}
)

## ==============================================================================
##
## Feedback on configuration settings
##
## ==============================================================================

if (LOFAR_VERBOSE_CONFIGURE)
  message (STATUS "-----------------------------------------------------------------")
  message (STATUS "[LOFAR] Configuration summary")
  message (STATUS "PACKAGE_NAME            = ${PACKAGE_NAME}")
  message (STATUS "LOFAR_SOURCE_DIR        = ${LOFAR_SOURCE_DIR}")
  message (STATUS "CMAKE_C_COMPILER        = ${CMAKE_C_COMPILER}")
  message (STATUS "CMAKE_CXX_COMPILER      = ${CMAKE_CXX_COMPILER}")
  message (STATUS "CCAS                    = ${CCAS}")
  message (STATUS "CXX_COMPILE_FLAGS       = ${CXX_COMPILE_FLAGS}")
  message (STATUS "CMAKE_MODULE_PATH       = ${CMAKE_MODULE_PATH}")
  message (STATUS "Have package MPI        = ${MPI_FOUND}")
  message (STATUS "Boost++                 = ${Boost_FOUND}")
  message (STATUS "Boost++ includes        = ${Boost_INCLUDE_DIR}")
  message (STATUS "Boost++ libraries       = ${Boost_LIBRARIES}")
  message (STATUS "CasaCore                = ${CASACORE_FOUND}")
  message (STATUS "Log4CPlus               = ${Log4CPlus_FOUND}")
  message (STATUS "SHMEM                   = ${HAVE_SHEM}")
  message (STATUS "Backtrace               = ${Backtrace_FOUND}")
  message (STATUS "-----------------------------------------------------------------")
endif (LOFAR_VERBOSE_CONFIGURE)
