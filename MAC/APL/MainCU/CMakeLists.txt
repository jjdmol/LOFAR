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
# Do not split the following line, otherwise makeversion will fail!
lofar_package(MainCU 1.0 DEPENDS Common OTDB MACIO GCFTM GCFPVSS GCFRTDB APLCommon RTDBCommon ApplCommon)

## ---------------------------------------------------------------------------
## External packages
## ---------------------------------------------------------------------------
include(LofarFindPackage)
lofar_find_package(Boost REQUIRED COMPONENTS date_time)

## ---------------------------------------------------------------------------
## Project-specific compiler definitions.
## ---------------------------------------------------------------------------
add_definitions(-DBOOST_DISABLE_THREADS)

## ---------------------------------------------------------------------------
## Generate configuration header file.
## ---------------------------------------------------------------------------
include(LofarConfig)

## ---------------------------------------------------------------------------
## Subdirectories
## ---------------------------------------------------------------------------
add_subdirectory(src)
