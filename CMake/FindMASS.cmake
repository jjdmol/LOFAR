# - Try to find MASS, the IBM BG/P Mathematical Acceleration Subsystem
# Variables used by this module:
#  MASS_ROOT_DIR     - MASS root directory
# Variables defined by this module:
#  MASS_FOUND        - system has MASS
#  MASS_INCLUDE_DIR  - the MASS include directory (cached)
#  MASS_INCLUDE_DIRS - the MASS include directories
#                      (identical to MASS_INCLUDE_DIR)
#  MASS_LIBRARY      - the scalar math library (cached)
#  MASSV_LIBRARY     - the vector math library (cached)
#  MASS_LIBRARIES    - all MASS libraries
#
# CAUTION: It is not guaranteed that the version of MASS found by this
# macro can be used on the BG/P, because we need to do cross-compilation.

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

if(NOT MASS_FOUND)

  find_path(MASS_INCLUDE_DIR massv.h
    PATHS ${MASS_ROOT_DIR} PATH_SUFFIXES include)
  find_library(MASS_LIBRARY mass
    PATHS ${MASS_ROOT_DIR} PATH_SUFFIXES lib)
  find_library(MASSV_LIBRARY massv
    PATHS ${MASS_ROOT_DIR} PATH_SUFFIXES lib)
  mark_as_advanced(MASS_INCLUDE_DIR MASS_LIBRARY MASSV_LIBRARY)

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(MASS DEFAULT_MSG
    MASS_LIBRARY MASSV_LIBRARY MASS_INCLUDE_DIR)

  set(MASS_INCLUDE_DIRS ${MASS_INCLUDE_DIR})
  set(MASS_LIBRARIES ${MASS_LIBRARY} ${MASSV_LIBRARY})

endif(NOT MASS_FOUND)
