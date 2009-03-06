## ------------------------------------------------------------------------------
## Project characteristics

project (AMC)

cmake_minimum_required (VERSION 2.6)

## ------------------------------------------------------------------------------
## Directories to be included in the build

## --- AMCBASE --------------------------------------

find_path (HAVE_AMCBASE CMakeLists.txt ${LOFAR_SOURCE_DIR}/LCS/AMC/AMCBase)

if (HAVE_AMCBASE)
  set (HAVE_AMCBASE TRUE CACHE BOOL "Have package AMCBase?" FORCE)
  add_subdirectory (AMCBase)
endif (HAVE_AMCBASE)  

## --- AMCIMPL -------------------------------------
find_path (HAVE_AMCIMPL CMakeLists.txt ${LOFAR_SOURCE_DIR}/LCS/AMC/AMCImpl)
if (HAVE_AMCIMPL)
  set (HAVE_AMCIMPL TRUE CACHE BOOL "Have package AMCImpl?" FORCE)
  add_subdirectory (AMCImpl)
endif (HAVE_AMCIMPL)  
