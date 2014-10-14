# - Try to find ALGLIB.
# Variables used by this module:
#  ALGLIB_ROOT_DIR     - ALGLIB root directory
# Variables defined by this module:
#  ALGLIB_FOUND        - system has ALGLIB
#  ALGLIB_INCLUDE_DIR  - the ALGLIB include directory (cached)
#  ALGLIB_INCLUDE_DIRS - the ALGLIB include directories
#                       (identical to ALGLIB_INCLUDE_DIR)
#  ALGLIB_LIBRARY      - the ALGLIB library (cached)
#  ALGLIB_LIBRARIES    - the ALGLIB libraries
#                       (identical to ALGLIB_LIBRARY)

# Copyright (C) 2009
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
# $Id: FindALGLIB.cmake 21886 2012-09-04 11:57:26Z mol $

if(NOT ALGLIB_FOUND)

  find_path(ALGLIB_INCLUDE_DIR bessel.h
    HINTS ${ALGLIB_ROOT_DIR} PATH_SUFFIXES include)
  find_library(ALGLIB_LIBRARY alglib
    HINTS ${ALGLIB_ROOT_DIR} PATH_SUFFIXES lib)
  mark_as_advanced(ALGLIB_INCLUDE_DIR ALGLIB_LIBRARY)

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(ALGLIB DEFAULT_MSG 
    ALGLIB_LIBRARY ALGLIB_INCLUDE_DIR)

  set(ALGLIB_INCLUDE_DIRS ${ALGLIB_INCLUDE_DIR})
  set(ALGLIB_LIBRARIES ${ALGLIB_LIBRARY})

endif(NOT ALGLIB_FOUND)
