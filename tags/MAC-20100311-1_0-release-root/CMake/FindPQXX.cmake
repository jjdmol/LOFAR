# - Try to find libpqxx: the official C++ client API for PostgreSQL
# Variables used by this module:
#  PQXX_ROOT_DIR     - PQXX root directory
#  PQ_ROOT_DIR       - PQ root directory (Postgres C API)
# Variables defined by this module:
#  PQXX_FOUND        - system has PQXX
#  PQXX_INCLUDE_DIR  - the PQXX include directory (cached)
#  PQXX_INCLUDE_DIRS - the PQXX include directories 
#                      (identical to PQXX_INCLUDE_DIR)
#  PQXX_LIBRARY      - the PQXX library (cached)
#  PQXX_LIBRARIES    - the PQXX libraries
#                      (identical to PQXX_LIBRARY)

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

if(NOT PQXX_FOUND)

  find_path(PQXX_INCLUDE_DIR pqxx/pqxx
    PATHS ${PQXX_ROOT_DIR} PATH_SUFFIXES include)
  find_library(PQXX_LIBRARY pqxx
    PATHS ${PQXX_ROOT_DIR} PATH_SUFFIXES lib)
  find_library(PQ_LIBRARY pq
    PATHS ${PQ_ROOT_DIR} PATH_SUFFIXES lib)
  mark_as_advanced(PQXX_INCLUDE_DIR PQXX_LIBRARY PQ_LIBRARY)

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(PQXX DEFAULT_MSG
    PQXX_LIBRARY PQ_LIBRARY PQXX_INCLUDE_DIR)

  set(PQXX_INCLUDE_DIRS ${PQXX_INCLUDE_DIR})
  set(PQXX_LIBRARIES ${PQXX_LIBRARY} ${PQ_LIBRARY})

endif(NOT PQXX_FOUND)
