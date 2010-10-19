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

# We need to create a symlink to the parmdb_main executable in the current
# binary directory, so that the test program(s) can find it.
get_target_property(_parmdb_main parmdb_main LOCATION_${CMAKE_BUILD_TYPE})
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${_parmdb_main}
  ${CMAKE_CURRENT_BINARY_DIR}/parmdb_main)

lofar_add_test(tpyparmdb DEPENDS parmdb_main)
