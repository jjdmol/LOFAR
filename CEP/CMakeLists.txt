# $Id$

cmake_minimum_required(VERSION 2.6)

## ---------------------------------------------------------------------------
## Preamble: set compilers
## ---------------------------------------------------------------------------
include(LofarInit)

## ---------------------------------------------------------------------------
## Project characteristics
## ---------------------------------------------------------------------------
project(CEP)
include(LofarGeneral)

## ---------------------------------------------------------------------------
## Subdirectories
## ---------------------------------------------------------------------------
lofar_add_package(MWCommon)
lofar_add_package(MS)
lofar_add_package(ParmDB)
lofar_add_package(Calibration)
lofar_add_package(DP3)
lofar_add_package(Imager)

lofar_add_package(Pipeline)
lofar_add_package(pyparmdb)

## ---------------------------------------------------------------------------
## Generate configuration header file.
## ---------------------------------------------------------------------------
include(LofarConfig)
