# $Id$

lofar_package(LofarFT 0.1 DEPENDS BBSKernel Common ParmDB ElementResponse)

lofar_find_package(Casarest REQUIRED COMPONENTS synthesis)
lofar_find_package(Casacore REQUIRED COMPONENTS images msfits)
lofar_find_package(Boost REQUIRED COMPONENTS thread)
lofar_find_package(FFTW3 REQUIRED COMPONENTS single double threads)

# Needed for casarest
add_definitions(-DCASA_STANDALONE)

add_subdirectory(include/LofarFT)
add_subdirectory(src)
add_subdirectory(test)
