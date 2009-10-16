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
lofar_package(IONProc 1.0
  DEPENDS Common Stream AMCBase AMCImpl PLC Interface FCNP)

## ---------------------------------------------------------------------------
## External packages
## ---------------------------------------------------------------------------
include(LofarFindPackage)
lofar_find_package(Boost REQUIRED)

## ---------------------------------------------------------------------------
## Enable BGP specific assembler
## ---------------------------------------------------------------------------
enable_language(ASM-BGP)

## ---------------------------------------------------------------------------
## Generate configuration header file.
## ---------------------------------------------------------------------------
include(LofarConfig)

## ---------------------------------------------------------------------------
## Subdirectories
## ---------------------------------------------------------------------------
add_subdirectory(src)
#add_subdirectory(test)    # Tests currently do NOT compile!
