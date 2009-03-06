## ------------------------------------------------------------------------------
## Project characteristics

project (ACC)

cmake_minimum_required (VERSION 2.6)

## ------------------------------------------------------------------------------
## Directories to be included in the build

## --- APS ------------------------------------------

find_path (HAVE_APS CMakeLists.txt ${LOFAR_SOURCE_DIR}/LCS/ACC/APS)

if (HAVE_APS)
  set (HAVE_APS TRUE CACHE BOOL "Have package APS?" FORCE)
  add_subdirectory (APS)
endif (HAVE_APS)  

## --- PLC ------------------------------------------

find_path (HAVE_PLC CMakeLists.txt ${LOFAR_SOURCE_DIR}/LCS/ACC/PLC)

if (HAVE_PLC)
  set (HAVE_PLC TRUE CACHE BOOL "Have package PLC?" FORCE)
  add_subdirectory (PLC)
endif (HAVE_PLC)  

