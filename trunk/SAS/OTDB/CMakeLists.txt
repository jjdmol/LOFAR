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
lofar_package(OTDB 1.0 DEPENDS Common)

## ---------------------------------------------------------------------------
## External packages
## ---------------------------------------------------------------------------
include(LofarFindPackage)
lofar_find_package(Boost REQUIRED COMPONENTS date_time)
lofar_find_package(PQXX REQUIRED)

## ---------------------------------------------------------------------------
## Generate configuration header file.
## ---------------------------------------------------------------------------
include(LofarConfig)

## ---------------------------------------------------------------------------
## Subdirectories
## ---------------------------------------------------------------------------
add_subdirectory(include/OTDB)
add_subdirectory(src)
add_subdirectory(test)
