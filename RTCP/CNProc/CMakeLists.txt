# $Id$

lofar_package(CNProc 1.0 DEPENDS Common Stream Interface FCNP)

include(LofarFindPackage)
lofar_find_package(MASS)
lofar_find_package(Boost REQUIRED)
lofar_find_package(FFTW3)
if(NOT FFTW3_F0UND)
  lofar_find_package(FFTW2)
  if(NOT FFTW2_FOUND)
    message(SEND_ERROR "Should have FFTW3 or FFTW2 installed.")
  endif(NOT FFTW2_FOUND)
endif(NOT FFTW3_F0UND)

enable_language(ASM-BGP)
set(CMAKE_C_LINK_EXECUTABLE ${CMAKE_ASM-BGP_LINK_EXECUTABLE})
set(CMAKE_CXX_LINK_EXECUTABLE ${CMAKE_ASM-BGP_LINK_EXECUTABLE})

add_subdirectory(src)
add_subdirectory(test)
