# - Try to find CPPLapack, a C++ class wrapper for BLAS and LAPACK.
# Variables used by this module:
#  CPPLAPACK_ROOT_DIR     - CPPLAPACK root directory
# Variables defined by this module:
#  CPPLAPACK_FOUND        - system has CPPLapack
#  CPPLAPACK_INCLUDE_DIR  - the CPPLapack include directory (cached)
#  CPPLAPACK_INCLUDE_DIRS - the CPPLapack include directories
#                           (identical to CPPLAPACK_INCLUDE_DIR)
#  CPPLAPACK_LIBRARIES    - libraries that CPPLapack depends upon

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

if(NOT CPPLAPACK_FOUND)

  find_path(CPPLAPACK_INCLUDE_DIR cpplapack.h
    HINTS ${CPPLAPACK_ROOT_DIR} PATH_SUFFIXES include)
  mark_as_advanced(CPPLAPACK_INCLUDE_DIR)

  if(CPPLAPACK_FIND_REQUIRED)
    find_package(LAPACK REQUIRED)
  else(CPPLAPACK_FIND_REQUIRED)
    find_package(LAPACK)
  endif(CPPLAPACK_FIND_REQUIRED)

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(CPPLapack DEFAULT_MSG
    CPPLAPACK_INCLUDE_DIR)

  set(CPPLAPACK_INCLUDE_DIRS ${CPPLAPACK_INCLUDE_DIR})
  set(CPPLAPACK_LIBRARIES ${LAPACK_LIBRARIES})

endif(NOT CPPLAPACK_FOUND)
