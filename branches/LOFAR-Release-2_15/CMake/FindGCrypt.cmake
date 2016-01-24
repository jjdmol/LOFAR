# - Try to find GCrypt: a crypto library
# This will define:
#  GCRYPT_FOUND        - system has GCrypt
#  GCRYPT_INCLUDE_DIR  - the GCrypt include directory (cached)
#  GCRYPT_INCLUDE_DIRS - the GCrypt include directories 
#                       (identical to GCRYPT_INCLUDE_DIR)
#  GCRYPT_LIBRARY      - the GCrypt library (cached)
#  GCRYPT_LIBRARIES    - the GCrypt libraries
#                       (identical to GCRYPT_LIBRARY)

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

if(NOT GCRYPT_FOUND)

  find_path(GCRYPT_INCLUDE_DIR gcrypt.h
    HINTS ${GCRYPT_ROOT_DIR} PATH_SUFFIXES include)
  find_library(GCRYPT_LIBRARY gcrypt
    HINTS ${GCRYPT_ROOT_DIR} PATH_SUFFIXES lib)
  mark_as_advanced(GCRYPT_INCLUDE_DIR GCRYPT_LIBRARY)

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(GCrypt DEFAULT_MSG
    GCRYPT_LIBRARY GCRYPT_INCLUDE_DIR)

  set(GCRYPT_INCLUDE_DIRS ${GCRYPT_INCLUDE_DIR})
  set(GCRYPT_LIBRARIES ${GCRYPT_LIBRARY})

endif(NOT GCRYPT_FOUND)
