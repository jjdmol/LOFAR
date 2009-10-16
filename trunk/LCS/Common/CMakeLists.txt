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
lofar_package(Common 3.1)

## ---------------------------------------------------------------------------
## External packages
## ---------------------------------------------------------------------------
include(LofarFindPackage)
lofar_find_package(Boost)
lofar_find_package(Readline)

## ---------------------------------------------------------------------------
## Generate configuration header file.
## ---------------------------------------------------------------------------
include(LofarConfig)

## ---------------------------------------------------------------------------
## Subdirectories
## ---------------------------------------------------------------------------
add_subdirectory(include/Common)
add_subdirectory(src)
add_subdirectory(test)
