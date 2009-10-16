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

lofar_add_test(tControllerDefines tControllerDefines.cc)
lofar_add_test(tAPLUtilities tAPLUtilities.cc)
lofar_add_test(tbitsetUtil tbitsetUtil.cc)
lofar_add_test(tOutOfBand tOutOfBand.cc)
lofar_add_test(tAntennaMapper tAntennaMapper.cc)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/ResourceAllocatorTest.log_prop.in
  ${CMAKE_CURRENT_BINARY_DIR}/ResourceAllocatorTest.log_prop)
