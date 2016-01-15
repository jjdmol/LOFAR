# $Id$

lofar_package(AWImager2 DEPENDS Common ParmDB StationResponse)

include(LofarFindPackage)

lofar_find_package(Casa COMPONENTS synthesis components)

if(${CASA_FOUND})
  lofar_find_package(Casacore REQUIRED COMPONENTS images msfits coordinates python)
  lofar_find_package(Boost REQUIRED COMPONENTS thread python system)
  lofar_find_package(FFTW3 REQUIRED COMPONENTS single double threads)
  lofar_find_package(Python REQUIRED)
  lofar_find_package(Pyrap REQUIRED)
  lofar_find_package(IDG)

  add_subdirectory(include/AWImager2)
  add_subdirectory(src)
  add_subdirectory(test)
else()
  message(WARNING "Casa was not found, NOT building AWImager2")
endif()
