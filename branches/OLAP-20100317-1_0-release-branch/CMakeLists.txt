# $Id$

cmake_minimum_required(VERSION 2.6)

## ---------------------------------------------------------------------------
## Preamble: set compilers
## ---------------------------------------------------------------------------
include(CMake/LofarInit.cmake)

## ---------------------------------------------------------------------------
## Project characteristics
## ---------------------------------------------------------------------------
project(LOFAR)
include(LofarGeneral)

## ---------------------------------------------------------------------------
## LOFAR packages: build all, unless BUILD_PACKAGES was specified
## ---------------------------------------------------------------------------
include(LofarPackage)
if(NOT DEFINED BUILD_PACKAGES)
  lofar_add_package(LCS)
  lofar_add_package(CEP)
  lofar_add_package(RTCP)
  lofar_add_package(SAS)
  lofar_add_package(MAC)
  lofar_add_package(LCU)
  lofar_add_package(SubSystems)
else(NOT DEFINED BUILD_PACKAGES)
  separate_arguments(BUILD_PACKAGES)
  foreach(pkg ${BUILD_PACKAGES})
    lofar_add_package(${pkg} REQUIRED)
  endforeach(pkg ${BUILD_PACKAGES})
endif(NOT DEFINED BUILD_PACKAGES)

## ---------------------------------------------------------------------------
## Generate configuration header file.
## ---------------------------------------------------------------------------
include(LofarConfig)
