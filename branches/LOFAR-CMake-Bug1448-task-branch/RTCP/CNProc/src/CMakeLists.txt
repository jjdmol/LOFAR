# $Id$

include(LofarPackageVersion)

# Create symbolic link to include directory.
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/include/${PACKAGE_NAME})

# Add current source directory to -I path. This is needed because CNProc uses
# angle brackets for internal header files, instead of quotes.
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(cnproc_LIB_SRCS
  Package__Version.cc
  AsyncCommunication.cc
  AsyncTranspose.cc
  BandPass.cc
  CN_Processing.cc
  Correlator.cc
  FCNP_ClientStream.cc
  FIR.cc
  LocationInfo.cc
  PencilBeams.cc
  PPF.cc
  Stokes.cc)

# Maybe we shoud use LOFAR_COMPILER_SUITE, because ASM-BGP_COMPILER_WORKS
# is not reliable: i.e.
#if(LOFAR_COMPILER_SUITE STREQUAL GNUBGP)
if(CMAKE_ASM-BGP_COMPILER_WORKS)
  list(APPEND cnproc_LIB_SRCS
    BeamFormerAsm.S
    CorrelatorAsm.S
    FIR_Asm.S
    FFT_Asm.S)
endif(CMAKE_ASM-BGP_COMPILER_WORKS)
#endif(LOFAR_COMPILER_SUITE STREQUAL GNUBGP)

lofar_add_library(cnproc ${cnproc_LIB_SRCS})

#message(STATUS "cnproc_LIB_SRCS = ${cnproc_LIB_SRCS}")
#message(STATUS "ASM-BGP = ${ASM-BGP}")
#message(STATUS "CMAKE_ASM-BGP_COMPILER_WORKS = ${CMAKE_ASM-BGP_COMPILER_WORKS}")
#message(STATUS "CMAKE_ASM-BGP_COMPILER = ${CMAKE_ASM-BGP_COMPILER}")
#message(STATUS "CMAKE_ASM-BGP_COMPILE_OBJECT = ${CMAKE_ASM-BGP_COMPILE_OBJECT}")
#message(STATUS "CMAKE_ASM-BGP_LINK_EXECUTABLE = ${CMAKE_ASM-BGP_LINK_EXECUTABLE}")

lofar_add_executable(CN_Processing CN_Processing_main.cc)
lofar_add_executable(versioncnproc versioncnproc.cc)

