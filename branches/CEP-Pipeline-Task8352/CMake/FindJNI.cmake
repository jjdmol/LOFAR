# - A tiny wrapper around the FindJNI.cmake macro that comes with CMake.
# Its purpose is twofold:
#  - Set JNI_FOUND if it's not set already. FindJNI.cmake that ships with
#    CMake 2.8.0 and earlier do not set this variable.
#  - If JNI is not found, then clear JNI_INCLUDE_DIRS and JNI_LIBRARIES.

# Copyright (C) 2010
# ASTRON (Netherlands Institute for Radio Astronomy)
# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
#
# This file is part of the LOFAR software suite.
# The LOFAR software suite is free software: you can redistribute it and/or
# modify it under the terms of the GNU General Public License as published
# by the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# The LOFAR software suite is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
#
# $Id$

# Call the "real" FindJNI module.
include(${CMAKE_ROOT}/Modules/FindJNI.cmake)

if("${CMAKE_VERSION}" VERSION_LESS "2.8.1")
  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(JNI DEFAULT_MSG
    JAVA_AWT_LIBRARY JAVA_JVM_LIBRARY
    JAVA_INCLUDE_PATH JAVA_INCLUDE_PATH2 JAVA_AWT_INCLUDE_PATH)
endif("${CMAKE_VERSION}" VERSION_LESS "2.8.1")

# Clear JNI_INCLUDE_DIRS and JNI_LIBRARIES if JNI was not found.
if(NOT JNI_FOUND)
  set(JNI_INCLUDE_DIRS)
  set(JNI_LIBRARIES)
endif(NOT JNI_FOUND)
