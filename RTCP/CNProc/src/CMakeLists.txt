#  $Id$
#
#  Copyright (C) 2008-2009
#  ASTRON (Netherlands Foundation for Research in Astronomy)
#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

include(LofarPackageVersion)

# Create symbolic link to include directory.
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/include/${PROJECT_NAME})

# Add current source directory to -I path. This is needed because CNProc uses
# angle brackets for internal header files, instead of quotes.
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(cnproc_LIB_SRCS
  Package__Version.cc
  AsyncCommunication.cc
  AsyncTranspose.cc
  BandPass.cc
  BeamFormer.cc
  CN_Processing.cc
  Correlator.cc
  FCNP_ClientStream.cc
  FIR.cc
  LocationInfo.cc
  PencilBeams.cc
  PPF.cc
  Stokes.cc)

if(CMAKE_ASM-BGP_COMPILER_WORKS)
#  set(CMAKE_ASM-BGP_LINK_EXECUTABLE "<CMAKE_LINKER> <FLAGS> <CMAKE_ASM${ASM_DIALECT}_LINK_FLAGS> <LINK_FLAGS> <OBJECTS>  -o <TARGET> <LINK_LIBRARIES>")
  list(APPEND cnproc_LIB_SRCS
    BeamFormerAsm.S
    CorrelatorAsm.S
    FIR_Asm.S
    FFT_Asm.S)
endif(CMAKE_ASM-BGP_COMPILER_WORKS)

lofar_add_library(cnproc ${cnproc_LIB_SRCS})

set(CMAKE_C_LINK_EXECUTABLE ${CMAKE_ASM-BGP_LINK_EXECUTABLE})
set(CMAKE_CXX_LINK_EXECUTABLE ${CMAKE_ASM-BGP_LINK_EXECUTABLE})

message(STATUS "cnproc_LIB_SRCS = ${cnproc_LIB_SRCS}")
message(STATUS "ASM-BGP = ${ASM-BGP}")
message(STATUS "CMAKE_ASM-BGP_COMPILER_WORKS = ${CMAKE_ASM-BGP_COMPILER_WORKS}")
message(STATUS "CMAKE_ASM-BGP_COMPILER = ${CMAKE_ASM-BGP_COMPILER}")
message(STATUS "CMAKE_ASM-BGP_COMPILE_OBJECT = ${CMAKE_ASM-BGP_COMPILE_OBJECT}")
message(STATUS "CMAKE_ASM-BGP_LINK_EXECUTABLE = ${CMAKE_ASM-BGP_LINK_EXECUTABLE}")

lofar_add_executable(CN_Processing CN_Processing_main.cc)
lofar_add_executable(versioncnproc versioncnproc.cc)

