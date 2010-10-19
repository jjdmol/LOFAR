# - Try to find valgrind headers
# Variables used by this module:
#  VALGRIND_ROOT_DIR    - Valgrind root directory
# Variables defined by this module:
#  VALGRIND_FOUND       - system has valgrind
#  VALGRIND_INCLUDE_DIR - the valgrind include directory (cached)
#  VALGRIND_INCLUDE_DIRS - the valgrind include directories
#                       (identical to VALGRIND_INCLUDE_DIR)
#  HAVE_VALGRIND        - True if system has Valgrind headers (cached)


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
# $Id: Findvalgrind.cmake 14630 2009-12-08 13:47:53Z loose $

if(NOT VALGRIND_FOUND)

  find_path(VALGRIND_INCLUDE_DIR valgrind/valgrind.h
    PATHS ${VALGRIND_ROOT_DIR} PATH_SUFFIXES include)
  mark_as_advanced(VALGRIND_INCLUDE_DIR)

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(Valgrind DEFAULT_MSG 
    VALGRIND_INCLUDE_DIR)

  set(VALGRIND_INCLUDE_DIRS ${VALGRIND_INCLUDE_DIR})

  set(HAVE_VALGRIND TRUE CACHE INTERNAL "Define if Valgrind headers are installed")

endif(NOT VALGRIND_FOUND)
