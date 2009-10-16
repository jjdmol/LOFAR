# $Id$

cmake_minimum_required(VERSION 2.6)

## ---------------------------------------------------------------------------
## Preamble: set compilers
## ---------------------------------------------------------------------------
include(LofarInit)

## ---------------------------------------------------------------------------
## Project characteristics
## ---------------------------------------------------------------------------
project(MAC)
include(LofarGeneral)

## ---------------------------------------------------------------------------
## Subdirectories
## ---------------------------------------------------------------------------
lofar_add_package(MACIO)       # MAC I/O related functions
lofar_add_package(GCF)         # Generic Control Framework
lofar_add_package(APL)         # Application subdirectory
lofar_add_package(Deployment)  # Deployment related functions
lofar_add_package(Navigator2)
lofar_add_package(MACTools Tools)

## ---------------------------------------------------------------------------
## Generate configuration header file.
## ---------------------------------------------------------------------------
include(LofarConfig)

