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

# Create symbolic link in binary include directory to current directory.
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/include/${PACKAGE_NAME})

# Add the directory where the generated C++ headers are stored to the -I path.
#include_directories(${CMAKE_BINARY_DIR}/include/MAC)

lofar_add_library(absdevel
  Package__Version.cc
  Beam.cc
  Beamlet.cc
  Beams.cc
  Beamlets.cc)

lofar_add_bin_program(versionbeam_server versionbeam_server.cc)
lofar_add_bin_program(BeamServer BeamServerMain.cc BeamServer.cc)
lofar_add_bin_program(beamctl beamctl.cc)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/BeamServer.conf.in
  ${CMAKE_CURRENT_BINARY_DIR}/BeamServer.conf)
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/beamctl.conf.in
  ${CMAKE_CURRENT_BINARY_DIR}/beamctl.conf)
  
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/BeamServer.conf
  ${CMAKE_CURRENT_BINARY_DIR}/beamctl.conf
  DESTINATION etc)
