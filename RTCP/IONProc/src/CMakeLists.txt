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
  ${CMAKE_BINARY_DIR}/include/${PACKAGE_NAME})

# Add current source directory to -I path. This is needed because IONProc uses
# angle brackets for internal header files, instead of quotes.
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

lofar_add_library(ionproc
  Package__Version.cc
  BeamletBuffer.cc
  BeamletBufferToComputeNode.cc
  CN_Personality.cc
  InputSection.cc
  InputThread.cc
  InputThreadAsm.S
  ION_Allocator.cc
  LogThread.cc
  OutputThread.cc
  OutputSection.cc
  ReaderWriterSynchronization.cc
  Scheduling.cc
  WH_DelayCompensation.cc
  FCNP_ServerStream.cc)

lofar_add_bin_program(versionionproc versionionproc.cc)
lofar_add_bin_program(ION_Processing ION_main.cc)
