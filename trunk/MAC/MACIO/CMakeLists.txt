# $Id$

cmake_minimum_required(VERSION 2.6)

## ---------------------------------------------------------------------------
## Preamble: set compilers
## ---------------------------------------------------------------------------
include(LofarInit)

## ---------------------------------------------------------------------------
## Project characteristics
## ---------------------------------------------------------------------------
include(LofarGeneral)

include(LofarPackage)
lofar_package(MACIO 1.0 DEPENDS Common)

## ---------------------------------------------------------------------------
## External packages
## ---------------------------------------------------------------------------
include(LofarFindPackage)
lofar_find_package(Boost)

## ---------------------------------------------------------------------------
## Add directory containing the generated MAC protocol headers to -I path.
## ---------------------------------------------------------------------------
include_directories(${CMAKE_BINARY_DIR}/include/MAC)

## ---------------------------------------------------------------------------
## Generate configuration header file.
## ---------------------------------------------------------------------------
include(LofarConfig)

## ---------------------------------------------------------------------------
## Subdirectories
## ---------------------------------------------------------------------------
add_subdirectory(autogen)
add_subdirectory(include/MACIO)
add_subdirectory(src)
add_subdirectory(test)
