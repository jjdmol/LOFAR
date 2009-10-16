# $Id$

cmake_minimum_required(VERSION 2.6)

## ---------------------------------------------------------------------------
## Preamble: set compilers
## ---------------------------------------------------------------------------
include(LofarInit)

## ---------------------------------------------------------------------------
## Project characteristics
## ---------------------------------------------------------------------------
project(GCF)
include(LofarGeneral)

## ---------------------------------------------------------------------------
## Create directory GCF in the binary include directory; it will hold symbolic
## links to the different subcomponents of GCF.
## ---------------------------------------------------------------------------
execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory
  ${CMAKE_BINARY_DIR}/include/GCF)

## ---------------------------------------------------------------------------
## Subdirectories
## ---------------------------------------------------------------------------
lofar_add_package(GCFTM   TM)     # Task Management
lofar_add_package(GCFPVSS PVSS)   # Low-level interface to PVSS database
lofar_add_package(GCFRTDB RTDB)   # Real-Time Database layer.

## ---------------------------------------------------------------------------
## Generate configuration header file.
## ---------------------------------------------------------------------------
include(LofarConfig)

