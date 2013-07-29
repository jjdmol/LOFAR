# - Try to find LIBHWLOC.
# Variables used by this module:
#  LIBHWLOC_ROOT_DIR     - LIBHWLOC root directory
# Variables defined by this module:
#  LIBHWLOC_FOUND        - system has LIBHWLOC
#  LIBHWLOC_INCLUDE_DIR  - the LIBHWLOC include directory (cached)
#  LIBHWLOC_INCLUDE_DIRS - the LIBHWLOC include directories
#                       (identical to LIBHWLOC_INCLUDE_DIR)
#  LIBHWLOC_LIBRARY      - the LIBHWLOC library (cached)
#  LIBHWLOC_LIBRARIES    - the LIBHWLOC libraries
#                       (identical to LIBHWLOC_LIBRARY)

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
# $Id: FindLIBHWLOC.cmake 21317 2012-06-26 14:22:47Z mol $

if(NOT LIBHWLOC_FOUND)

  find_path(LIBHWLOC_INCLUDE_DIR hwloc.h
    HINTS ${LIBHWLOC_ROOT_DIR} PATH_SUFFIXES include)
  find_library(LIBHWLOC_LIBRARY hwloc
    HINTS ${LIBHWLOC_ROOT_DIR} PATH_SUFFIXES lib)
  mark_as_advanced(LIBHWLOC_INCLUDE_DIR LIBHWLOC_LIBRARY)

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(LIBHWLOC DEFAULT_MSG 
    LIBHWLOC_LIBRARY LIBHWLOC_INCLUDE_DIR)

  set(LIBHWLOC_INCLUDE_DIRS ${LIBHWLOC_INCLUDE_DIR})
  set(LIBHWLOC_LIBRARIES ${LIBHWLOC_LIBRARY})

endif(NOT LIBHWLOC_FOUND)
