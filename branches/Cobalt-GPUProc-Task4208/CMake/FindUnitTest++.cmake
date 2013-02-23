# - Try to find UnitTest++.
# Variables used by this module:
#  UNITTEST++_ROOT_DIR     - UnitTest++ root directory
# Variables defined by this module:
#  UNITTEST++_FOUND        - system has UnitTest++
#  UNITTEST++_INCLUDE_DIR  - the UnitTest++ include directory (cached)
#  UNITTEST++_INCLUDE_DIRS - the UnitTest++ include directories
#                            (identical to UNITTEST++_INCLUDE_DIR)
#  UNITTEST++_LIBRARY      - the UnitTest++ library (cached)
#  UNITTEST++_LIBRARIES    - the UnitTest++ libraries
#                            (identical to UNITTEST++_LIBRARY)

# Copyright (C) 2013
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

if(NOT UNITTEST++_FOUND)

  find_path(UNITTEST++_INCLUDE_DIR UnitTest++.h
    HINTS ${UNITTEST++_ROOT_DIR} PATH_SUFFIXES unittest++)
  find_library(UNITTEST++_LIBRARY UnitTest++
    HINTS ${UNITTEST++_ROOT_DIR} PATH_SUFFIXES lib)
  mark_as_advanced(UNITTEST++_INCLUDE_DIR UNITTEST++_LIBRARY)

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(UnitTest++ DEFAULT_MSG 
    UNITTEST++_LIBRARY UNITTEST++_INCLUDE_DIR)

  set(UNITTEST++_INCLUDE_DIRS ${UNITTEST++_INCLUDE_DIR})
  set(UNITTEST++_LIBRARIES ${UNITTEST++_LIBRARY})

endif(NOT UNITTEST++_FOUND)
