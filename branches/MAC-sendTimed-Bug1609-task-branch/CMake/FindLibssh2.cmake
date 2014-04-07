# - Try to find libssh2.
# Variables used by this module:
#  LIBSSH2_ROOT_DIR     - LIBSSH2 root directory
# Variables defined by this module:
#  LIBSSH2_FOUND        - system has LIBSSH2
#  LIBSSH2_INCLUDE_DIR  - the LIBSSH2 include directory (cached)
#  LIBSSH2_INCLUDE_DIRS - the LIBSSH2 include directories
#                       (identical to LIBSSH2_INCLUDE_DIR)
#  LIBSSH2_LIBRARY      - the LIBSSH2 library (cached)
#  LIBSSH2_LIBRARIES    - the LIBSSH2 libraries
#                       (identical to LIBSSH2_LIBRARY)

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
# $Id: FindLIBSSH2.cmake 21317 2012-06-26 14:22:47Z mol $

if(NOT LIBSSH2_FOUND)

  find_path(LIBSSH2_INCLUDE_DIR libssh2.h
    HINTS ${LIBSSH2_ROOT_DIR} PATH_SUFFIXES include)
  find_library(LIBSSH2_LIBRARY ssh2
    HINTS ${LIBSSH2_ROOT_DIR} PATH_SUFFIXES lib)
  mark_as_advanced(LIBSSH2_INCLUDE_DIR LIBSSH2_LIBRARY)

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(LIBSSH2 DEFAULT_MSG 
    LIBSSH2_LIBRARY LIBSSH2_INCLUDE_DIR)

  set(LIBSSH2_INCLUDE_DIRS ${LIBSSH2_INCLUDE_DIR})
  set(LIBSSH2_LIBRARIES ${LIBSSH2_LIBRARY})

endif(NOT LIBSSH2_FOUND)
