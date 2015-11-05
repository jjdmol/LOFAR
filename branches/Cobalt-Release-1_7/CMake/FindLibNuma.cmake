# - Try to find libnuma.
# Variables used by this module:
#  LIBNUMA_ROOT_DIR     - LIBNUMA root directory
# Variables defined by this module:
#  LIBNUMA_FOUND        - system has LIBNUMA
#  LIBNUMA_INCLUDE_DIR  - the LIBNUMA include directory (cached)
#  LIBNUMA_INCLUDE_DIRS - the LIBNUMA include directories
#                       (identical to LIBNUMA_INCLUDE_DIR)
#  LIBNUMA_LIBRARY      - the LIBNUMA library (cached)
#  LIBNUMA_LIBRARIES    - the LIBNUMA libraries
#                       (identical to LIBNUMA_LIBRARY)

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
# $Id$

if(NOT LIBNUMA_FOUND)

  find_path(LIBNUMA_INCLUDE_DIR numa.h
    HINTS ${LIBNUMA_ROOT_DIR} PATH_SUFFIXES include)
  find_library(LIBNUMA_LIBRARY numa
    HINTS ${LIBNUMA_ROOT_DIR} PATH_SUFFIXES lib lib64)
  mark_as_advanced(LIBNUMA_INCLUDE_DIR LIBNUMA_LIBRARY)

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(LIBNUMA DEFAULT_MSG 
    LIBNUMA_LIBRARY LIBNUMA_INCLUDE_DIR)

  set(LIBNUMA_INCLUDE_DIRS ${LIBNUMA_INCLUDE_DIR})
  set(LIBNUMA_LIBRARIES ${LIBNUMA_LIBRARY})

endif(NOT LIBNUMA_FOUND)
