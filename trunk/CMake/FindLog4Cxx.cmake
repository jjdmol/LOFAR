# - Try to find Log4Cxx.
# Variables used by this module:
#  LOG4CXX_ROOT_DIR     - Log4Cxx root directory
# Variables defined by this module:
#  LOG4CXX_FOUND        - system has Log4Cxx
#  LOG4CXX_INCLUDE_DIR  - the Log4Cxx include directory (cached)
#  LOG4CXX_INCLUDE_DIRS - the Log4Cxx include directories
#                         (identical to LOG4CXX_INCLUDE_DIR)
#  LOG4CXX_LIBRARY      - the Log4Cxx library (cached)
#  LOG4CXX_LIBRARIES    - the Log4Cxx libraries
#                         (identical to LOG4CXX_LIBRARY)

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

if(NOT LOG4CXX_FOUND)

  find_path(LOG4CXX_INCLUDE_DIR log4cxx/logger.h
    HINTS ${LOG4CXX_ROOT_DIR} PATH_SUFFIXES include)
  find_library(LOG4CXX_LIBRARY log4cxx
    HINTS ${LOG4CXX_ROOT_DIR} PATH_SUFFIXES lib)
  mark_as_advanced(LOG4CXX_INCLUDE_DIR LOG4CXX_LIBRARY)

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(Log4Cxx DEFAULT_MSG
    LOG4CXX_LIBRARY LOG4CXX_INCLUDE_DIR)

  set(LOG4CXX_INCLUDE_DIRS ${LOG4CXX_INCLUDE_DIR})
  set(LOG4CXX_LIBRARIES ${LOG4CXX_LIBRARY})

endif(NOT LOG4CXX_FOUND)
