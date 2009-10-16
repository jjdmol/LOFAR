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

# Add the directory where the generated C++ headers are stored to the -I path.
include_directories(${PACKAGE_BINARY_DIR}/src)

# Add the project's source directory to the -I path.
include_directories(${PACKAGE_SOURCE_DIR}/src)

lofar_add_library(tbbstub StubRawEvent.cc)
lofar_add_sbin_program(TPStub TPStub.cc)
lofar_add_sbin_program(TBBTest TBBTest.cc)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/TPStub.conf.in
  ${CMAKE_CURRENT_BINARY_DIR}/TPStub.conf)
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/TBBTest.conf.in
  ${CMAKE_CURRENT_BINARY_DIR}/TBBTest.conf)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/TPStub.conf
  ${CMAKE_CURRENT_BINARY_DIR}/TBBTest.conf
  DESTINATION etc)
