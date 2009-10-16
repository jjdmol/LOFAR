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
lofar_package(CNProc 1.0 DEPENDS Common Stream Interface FCNP)

## ---------------------------------------------------------------------------
## External packages
## ---------------------------------------------------------------------------
include(LofarFindPackage)
lofar_find_package(MASS)
lofar_find_package(Boost REQUIRED)
lofar_find_package(FFTW3)
if(NOT FFTW3_F0UND)
  lofar_find_package(FFTW2)
  if(NOT FFTW2_FOUND)
    message(SEND_ERROR "Should have FFTW3 or FFTW2 installed.")
  endif(NOT FFTW2_FOUND)
endif(NOT FFTW3_F0UND)

## ---------------------------------------------------------------------------
## Enable BGP specific assembler.
## Use the BGP assembler also for linking C/C++ programs.
##
## NOTE: Maybe this should be moved to the variants file. That requires that
## CN and ION are no longer variants; we then need two compiler definitions:
## BGPCN and BGPION, which is a cleaner solution anyway.
## ---------------------------------------------------------------------------
enable_language(ASM-BGP)
set(CMAKE_C_LINK_EXECUTABLE ${CMAKE_ASM-BGP_LINK_EXECUTABLE})
set(CMAKE_CXX_LINK_EXECUTABLE ${CMAKE_ASM-BGP_LINK_EXECUTABLE})

## ---------------------------------------------------------------------------
## Generate configuration header file.
## ---------------------------------------------------------------------------
include(LofarConfig)

## ---------------------------------------------------------------------------
## Subdirectories
## ---------------------------------------------------------------------------
add_subdirectory(src)
add_subdirectory(test)
