## ------------------------------------------------------------------------------
## Project characteristics

project (RTCP)

cmake_minimum_required (VERSION 2.6)

## ------------------------------------------------------------------------------
## Set CMAKE_MODULE_PATH to load custom CMake modules

if (NOT LOFAR_ROOT)
find_path (LOFAR_ROOT cmake/modules/CMakeSettings.cmake
  PATHS 
  ${LCS_SOURCE_DIR}
  ${LCS_SOURCE_DIR}/..
  ${LCS_SOURCE_DIR}/../..
  ${LCS_SOURCE_DIR}/../../..
  $ENV{LOFARSOFT}
  )

if (LOFAR_ROOT)
  include (${LOFAR_ROOT}/cmake/CMakeSettings.cmake)
else (LOFAR_ROOT)
  message (FATAL_ERROR "Unable to locate additional CMake scripts!")
endif (LOFAR_ROOT)
endif (NOT LOFAR_ROOT)

## ------------------------------------------------------------------------------
## Additional CMake modules

include (CheckIncludeFiles)
include (CheckLibraryExists)
include (CheckTypeSize)
include (CTest)
enable_testing()

## Custom modules

#include (MakePackageVersion)

## ------------------------------------------------------------------------------
## Check system libraries

## ------------------------------------------------------------------------------
## Search for external libraries

## ------------------------------------------------------------------------------
## Check for required tools

## ------------------------------------------------------------------------------
## Module header files


## ------------------------------------------------------------------------------
## Directories to be included in the build

## --- Interface ------------------------------------

find_path (HAVE_INTERFACE CMakeLists.txt ${LOFAR_SOURCE_DIR}/RTCP/Interface)
if (HAVE_INTERFACE)
  set (HAVE_INTERFACE TRUE CACHE BOOL "Have package Interface?" FORCE)
  add_subdirectory (Interface)
endif (HAVE_INTERFACE)  

## --- FCNP -----------------------------------------

find_path (HAVE_FCNP CMakeLists.txt ${LOFAR_SOURCE_DIR}/RTCP/FCNP)
if (HAVE_FCNP)
  set (HAVE_FCNP TRUE CACHE BOOL "Have package FCNP?" FORCE)
  add_subdirectory (FCNP)
endif (HAVE_FCNP)  

## --- Storage --------------------------------------

find_path (HAVE_STORAGE CMakeLists.txt ${LOFAR_SOURCE_DIR}/RTCP/Storage)
if (HAVE_STORAGE)
  set (HAVE_STORAGE TRUE CACHE BOOL "Have package Storage?" FORCE)
  add_subdirectory (Storage)
endif (HAVE_STORAGE)  

## --- IONProc --------------------------------------

find_path (HAVE_IONPROC CMakeLists.txt ${LOFAR_SOURCE_DIR}/RTCP/IONProc)
if (HAVE_IONPROC)
  set (HAVE_IONPROC TRUE CACHE BOOL "Have package IONProc?" FORCE)
  add_subdirectory (IONProc)
endif (HAVE_IONPROC)  

## --- CNProc ---------------------------------------

find_path (HAVE_CNPROC CMakeLists.txt ${LOFAR_SOURCE_DIR}/RTCP/CNProc)
if (HAVE_CNPROC)
  set (HAVE_CNPROC TRUE CACHE BOOL "Have package CNProc?" FORCE)
  add_subdirectory (CNProc)
endif (HAVE_CNPROC)  

#add_subdirectory (CS1_Tools)
