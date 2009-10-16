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
lofar_package(MWCommon 0.1 DEPENDS Common Blob)

## ---------------------------------------------------------------------------
## External packages
## ---------------------------------------------------------------------------
include(LofarFindPackage)
lofar_find_package(Boost REQUIRED)
lofar_find_package(Casacore COMPONENTS casa REQUIRED)

## ---------------------------------------------------------------------------
## Generate configuration header file.
## ---------------------------------------------------------------------------
include(LofarConfig)

## ---------------------------------------------------------------------------
## Subdirectories
## ---------------------------------------------------------------------------
add_subdirectory(include/MWCommon)
add_subdirectory(src)
add_subdirectory(test)
