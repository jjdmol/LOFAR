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
lofar_package(CURTDBDaemons 1.0 DEPENDS Common ApplCommon MACIO GCFTM GCFRTDB RTDBCommon APLCommon OTDB)

## ---------------------------------------------------------------------------
## External packages
## ---------------------------------------------------------------------------
include(LofarFindPackage)
lofar_find_package(Boost REQUIRED COMPONENTS date_time)
lofar_find_package(PQXX REQUIRED)

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
add_subdirectory(test)
