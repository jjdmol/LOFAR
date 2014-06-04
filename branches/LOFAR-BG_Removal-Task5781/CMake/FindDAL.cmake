# - Try to find DAL.
# Variables used by this module:
#  DAL_ROOT_DIR     - DAL root directory
# Variables defined by this module:
#  DAL_FOUND        - system has DAL
#  DAL_INCLUDE_DIR  - the DAL include directory (cached)
#  DAL_INCLUDE_DIRS - the DAL include directories
#                       (identical to DAL_INCLUDE_DIR)
#  DAL_LIBRARY      - the DAL library (cached)
#  DAL_LIBRARIES    - the DAL libraries
#                       (identical to DAL_LIBRARY)

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

if(NOT DAL_FOUND)

  find_path(DAL_INCLUDE_DIR dal/dal_config.h
    HINTS ${DAL_ROOT_DIR} PATH_SUFFIXES include)
  find_library(DAL_LIBRARY lofardal
    HINTS ${DAL_ROOT_DIR} PATH_SUFFIXES lib)
  mark_as_advanced(DAL_INCLUDE_DIR DAL_LIBRARY)

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(DAL DEFAULT_MSG 
    DAL_LIBRARY DAL_INCLUDE_DIR)

  set(DAL_INCLUDE_DIRS ${DAL_INCLUDE_DIR})
  set(DAL_LIBRARIES ${DAL_LIBRARY})

endif(NOT DAL_FOUND)
