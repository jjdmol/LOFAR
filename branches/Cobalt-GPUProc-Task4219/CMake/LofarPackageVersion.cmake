# - Generate CMake script to create/update LOFAR package version information.
#
# This script generates a CMake script file UpdatePackageVersion.cmake that is
# used in the custom target <pkg>_PackageVersion to create or update the
# source files Package__Version.h, Package__Version.cc and version<pkg>.cc,
# which provide version information about the current LOFAR package. The
# custom target is considered to be always out-of-date, so that it will be
# rebuilt with each invocation of 'make'. It collects version information from
# the Subversion repository, using 'svn info' and 'svn status'. This is
# information is then used to create or update the files Package__Version.h
# Package__Version.cc and version<pkg>.cc, if necessary.
#
# NOTE: you still have to add these sources manually as argument to
# add_library() or add_executable().
#
# See also UpdatePackageVersion.cmake.in

#  $Id$
#
#  Copyright (C) 2010
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

configure_file(
  ${LOFAR_ROOT}/CMake/UpdatePackageVersion.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/UpdatePackageVersion.cmake @ONLY)

add_custom_target(${PACKAGE_NAME}_PackageVersion # ALL
  COMMAND ${CMAKE_COMMAND} 
  ARGS -P ${CMAKE_CURRENT_BINARY_DIR}/UpdatePackageVersion.cmake)

# Mark files Package__Version.cc and version<pkg>.cc as generated;
# otherwise CMake will complain it cannot find the source files.
string(TOLOWER ${PACKAGE_NAME} _lpkg)
set_source_files_properties(Package__Version.cc version${_lpkg}.cc
  PROPERTIES GENERATED ON)

# Add the path to the generated file Package__Version.h to the include path.
include_directories(${CMAKE_BINARY_DIR}/include/Package__Version)
