## ------------------------------------------------------------------------------
## Project characteristics

project (LCS)

cmake_minimum_required (VERSION 2.6)

##-------------------------------------------------------------------------------
## Options

option (LCS_BUILD_TESTS "Build test programs?" FALSE)

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

## --- Tools ----------------------------------------

find_path (HAVE_TOOLS CMakeLists.txt ${LOFAR_SOURCE_DIR}/LCS/Tools)
if (HAVE_TOOLS)
  set (HAVE_TOOLS TRUE CACHE BOOL "Have package Tools?" FORCE)
  add_subdirectory (Tools)
endif (HAVE_TOOLS)  

## --- Common ---------------------------------------

find_path (HAVE_COMMON CMakeLists.txt ${LOFAR_SOURCE_DIR}/LCS/Common)
if (HAVE_COMMON)
  set (HAVE_COMMON TRUE CACHE BOOL "Have package Common?" FORCE)
  add_subdirectory (Common)
endif (HAVE_COMMON)  

## --- ACC ------------------------------------------

find_path (HAVE_ACC CMakeLists.txt ${LOFAR_SOURCE_DIR}/LCS/ACC)
if (HAVE_ACC)
  set (HAVE_ACC TRUE CACHE BOOL "Have package ACC?" FORCE)
  add_subdirectory (ACC)
endif (HAVE_ACC)  

## --- AMC ------------------------------------------

find_path (HAVE_AMC CMakeLists.txt ${LOFAR_SOURCE_DIR}/LCS/AMC)
if (HAVE_AMC)
  set (HAVE_AMC TRUE CACHE BOOL "Have package AMC?" FORCE)
  add_subdirectory (AMC)
endif (HAVE_AMC)  

## --- BLOB -----------------------------------------

find_path (HAVE_BLOB CMakeLists.txt ${LOFAR_SOURCE_DIR}/LCS/Blob)
if (HAVE_BLOB)
  set (HAVE_BLOB TRUE CACHE BOOL "Have package Blob?" FORCE)
  add_subdirectory (Blob)
endif (HAVE_BLOB)  

## --- TRANSPORT ------------------------------------

find_path (HAVE_TRANSPORT CMakeLists.txt ${LOFAR_SOURCE_DIR}/LCS/Transport)
if (HAVE_TRANSPORT)
  set (HAVE_TRANSPORT TRUE CACHE BOOL "Have package Transport?" FORCE)
  add_subdirectory (Transport)
endif (HAVE_TRANSPORT)  

## --- STREAM ---------------------------------------

find_path (HAVE_STREAM CMakeLists.txt ${LOFAR_SOURCE_DIR}/LCS/Stream)
if (HAVE_STREAM)
  set (HAVE_STREAM TRUE CACHE BOOL "Have package Stream?" FORCE)
  add_subdirectory (Stream)
endif (HAVE_STREAM)  

## --- APPLCOMMON -----------------------------------

find_path (HAVE_APPLCOMMON CMakeLists.txt ${LOFAR_SOURCE_DIR}/LCS/ApplCommon)
if (HAVE_APPLCOMMON)
  set (HAVE_APPLCOMMON TRUE CACHE BOOL "Have package ApplCommon?" FORCE)
  add_subdirectory (ApplCommon)
endif (HAVE_APPLCOMMON)  

