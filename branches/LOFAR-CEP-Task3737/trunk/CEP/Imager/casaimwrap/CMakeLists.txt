# $Id$

lofar_package(casaimwrap 1.0 DEPENDS LofarFT)

include(LofarFindPackage)

lofar_find_package(Casarest REQUIRED COMPONENTS msvis synthesis)
lofar_find_package(Casacore REQUIRED COMPONENTS coordinates images ms msfits)
lofar_find_package(Boost REQUIRED COMPONENTS python thread)
lofar_find_package(FFTW3 REQUIRED COMPONENTS single double)
lofar_find_package(Python REQUIRED)
lofar_find_package(Pyrap REQUIRED)

# Needed for casarest
add_definitions(-DCASA_STANDALONE)

add_subdirectory(src)
#add_subdirectory(test)
