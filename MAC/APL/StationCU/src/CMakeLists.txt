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

# Add definition of include path suffix definition, because the header files
# reside in a non-standard location.
set(${PACKAGE_NAME}_INCLUDE_PATH_SUFFIX APL/ClockProtocol
  CACHE INTERNAL "Include path suffix for package \"${PACKAGE_NAME}\"")

# Create symbolic link in binary include directory to current directory.
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/include/StationCU)

lofar_add_library(stationcu Package__Version.cc)
lofar_add_bin_program(versionstationcu versionstationcu.cc)

add_subdirectory(BeamControl)
add_subdirectory(CalibrationControl)
add_subdirectory(ClockControl)
add_subdirectory(ClockProtocol)
add_subdirectory(HardwareMonitor)
add_subdirectory(StationControl)
add_subdirectory(TBBControl)
add_subdirectory(TempControl)
