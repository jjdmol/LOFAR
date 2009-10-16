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

include(LofarCTest)

# Add project's source directory to -I path.
include_directories(${PACKAGE_SOURCE_DIR}/src)

lofar_add_test(tCableAttenuation 
  tCableAttenuation.cc
  ${PACKAGE_SOURCE_DIR}/src/CableAttenuation.cc)
  
lofar_add_test(tRCUCables
  tRCUCables.cc
  ${PACKAGE_SOURCE_DIR}/src/RCUCables.cc
  ${PACKAGE_SOURCE_DIR}/src/CableAttenuation.cc)

lofar_add_sbin_program(EPAStub EPAStub.cc)
lofar_add_sbin_program(RSPTest RSPTest.cc)

install(PROGRAMS
  beamtest
  DESTINATION sbin)

foreach(file RSPTest.conf EPAStub.conf EPAStub.log_prop)
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/${file}.in
    ${CMAKE_CURRENT_BINARY_DIR}/${file})
  install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/${file}
    DESTINATION etc)
endforeach(file)
