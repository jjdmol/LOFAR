# $Id$

cmake_minimum_required(VERSION 2.6)

## ---------------------------------------------------------------------------
## Preamble: set compilers
## ---------------------------------------------------------------------------
include(LofarInit)

## ---------------------------------------------------------------------------
## Project characteristics
## ---------------------------------------------------------------------------
project(StationTest NONE)

## ---------------------------------------------------------------------------
## Install files matching regex pattern in current directory and below
## ---------------------------------------------------------------------------
install(DIRECTORY . 
  DESTINATION stationtest
  USE_SOURCE_PERMISSIONS
  FILES_MATCHING REGEX "(README\\.txt|\\.(gold|m|py|sh))$"
  PATTERN ".svn" EXCLUDE)
