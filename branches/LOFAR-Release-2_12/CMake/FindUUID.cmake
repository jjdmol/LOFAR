# - Try to find libuuid: A library to generate UUIDs
# Variables used by this module:
#  UUID_ROOT_DIR     - UUID root directory
# Variables defined by this module:
#  UUID_FOUND        - system has UUID
#  UUID_INCLUDE_DIR  - the UUID include directory (cached)
#  UUID_INCLUDE_DIRS - the UUID include directories
#                        (identical to UUID_INCLUDE_DIR)
#  UUID_LIBRARY      - the UUID library (cached)
#  UUID_LIBRARIES    - the UUID libraries
#                        (identical to UUID_LIBRARY)

# Copyright (C) 2015
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

if(NOT UUID_FOUND)

  find_path(UUID_INCLUDE_DIR uuid/uuid.h
    HINTS ${UUID_ROOT_DIR} PATH_SUFFIXES include)
  find_library(UUID_LIBRARY uuid
    HINTS ${UUID_ROOT_DIR} PATH_SUFFIXES lib)

  mark_as_advanced(UUID_INCLUDE_DIR UUID_LIBRARY)

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(UUID DEFAULT_MSG
    UUID_LIBRARY UUID_INCLUDE_DIR)

  set(UUID_INCLUDE_DIRS ${UUID_INCLUDE_DIR})
  set(UUID_LIBRARIES ${UUID_LIBRARY})

endif(NOT UUID_FOUND)
