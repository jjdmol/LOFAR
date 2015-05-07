# - Try to find Pyrap.
#
# Find module(s) used by this module:
#  FindCasacore
#
# Variables used by this module:
#  PYRAP_ROOT_DIR         - Pyrap root directory     (Pyrap 1.x)
#  CASACORE_ROOT_DIR      - Casacore root directory  (Pyrap 2.x)
#
# Variables defined by this module:
#  PYRAP_FOUND            - system has Pyrap
#  PYRAP_INCLUDE_DIR      - the Pyrap 1.x include directory (cached)
#  PYCASACORE_INCLUDE_DIR - the Pyrap 2.x include directory (cached)
#  PYRAP_INCLUDE_DIRS     - the Pyrap include directories
#                           (identical to PYRAP_INCLUDE_DIR (pyrap 1.x); or
#                            identical to CASACORE_INCLUDE_DIRS (pyrap 2.x))
#  PYRAP_LIBRARY          - the Pyrap 1.x library (cached)
#  PYRAP_LIBRARIES        - the Pyrap libraries
#                           (identical to PYRAP_LIBRARY (pyrap 1.x only), 
#                            plus CASACORE_LIBRARIES)

# Copyright (C) 2009-2015
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


# Helper macro to find casacore components that pyrap depends on.
macro(_pyrap_find_casacore)
  message(STATUS "Looking for Pyrap casacore dependencies")
  include(LofarFindPackage)
  if(Pyrap_FIND_REQUIRED)
    lofar_find_package(Casacore REQUIRED COMPONENTS ${ARGN})
  else(Pyrap_FIND_REQUIRED)
    lofar_find_package(Casacore COMPONENTS ${ARGN})
  endif(Pyrap_FIND_REQUIRED)
endmacro(_pyrap_find_casacore)


if(NOT PYRAP_FOUND)

  # First try to find pyrap
  find_path(PYRAP_INCLUDE_DIR pyrap/Converters.h
    HINTS ${PYRAP_ROOT_DIR} PATH_SUFFIXES include)

  if (PYRAP_INCLUDE_DIR)
  
    # Found old pyrap
    find_library(PYRAP_LIBRARY pyrap
      HINTS ${PYRAP_ROOT_DIR} PATH_SUFFIXES lib)
    mark_as_advanced(PYRAP_INCLUDE_DIR PYRAP_LIBRARY)

    _pyrap_find_casacore(casa)

    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(Pyrap DEFAULT_MSG 
      PYRAP_LIBRARY PYRAP_INCLUDE_DIR)

    set(PYRAP_INCLUDE_DIRS ${PYRAP_INCLUDE_DIR} ${CASACORE_INCLUDE_DIRS})
    set(PYRAP_LIBRARIES ${PYRAP_LIBRARY} ${CASACORE_LIBRARIES})

  endif(PYRAP_INCLUDE_DIR)

endif(NOT PYRAP_FOUND)


if(NOT PYRAP_FOUND)

  _pyrap_find_casacore(casa python)

  find_path(PYCASACORE_INCLUDE_DIR casacore/python/Converters.h
    HINTS ${CASACORE_INCLUDE_DIR} PATH_SUFFIXES include)

  mark_as_advanced(PYCASACORE_INCLUDE_DIR)

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(Pyrap DEFAULT_MSG 
    PYCASACORE_INCLUDE_DIR)

  set(PYRAP_INCLUDE_DIRS ${CASACORE_INCLUDE_DIRS})
  set(PYRAP_LIBRARIES ${CASACORE_LIBRARIES})

endif(NOT PYRAP_FOUND)
