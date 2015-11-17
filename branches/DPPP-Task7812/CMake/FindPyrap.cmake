# - Try to find Pyrap.
#
# Pyrap provides python bindings to the casacore radio astronomy libraries.
# With the release of casacore 2.0, pyrap has been superseded by 
# python-casacore. This find module remains for backward compatibility.
#
# Find module(s) used by this module:
#  FindCasacore
#
# Variables used by this module:
#  PYRAP_ROOT_DIR     - Pyrap root directory
#
# Variables defined by this module:
#  PYRAP_FOUND        - system has Pyrap
#  PYRAP_INCLUDE_DIR  - the Pyrap include directory (cached)
#  PYRAP_INCLUDE_DIRS - the Pyrap include directories
#                       (identical to PYRAP_INCLUDE_DIR)
#  PYRAP_LIBRARY      - the Pyrap library (cached)
#  PYRAP_LIBRARIES    - the Pyrap libraries
#                       (identical to PYRAP_LIBRARY)

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

include(LofarFindPackage)

if(NOT PYRAP_FOUND)
  # Casacore 2.x has the Python converters built-in; try this first.
  lofar_find_package(Casacore COMPONENTS python)

  if(CASA_PYTHON_LIBRARY)
    # We found the cascore python component. 
    # Set PYRAP_INCLUDE_DIR and PYRAP_LIBRARY for backward compatibility.
    set(PYRAP_INCLUDE_DIR ${CASACORE_INCLUDE_DIR}
      CACHE PATH "Path to a file.")
    set(PYRAP_LIBRARY ${CASA_PYTHON_LIBRARY}
      CACHE FILEPATH "Path to a library.")
  else()
    # Try to find the old pyrap.
    find_path(PYRAP_INCLUDE_DIR pyrap/Converters.h
      HINTS ${PYRAP_ROOT_DIR} PATH_SUFFIXES include)
    find_library(PYRAP_LIBRARY pyrap
      HINTS ${PYRAP_ROOT_DIR} PATH_SUFFIXES lib)
    # Pyrap also depends on the casa library in casacore.
    if(Pyrap_FIND_REQUIRED)
      lofar_find_package(Casacore REQUIRED COMPONENTS casa)
    else()
      lofar_find_package(Casacore COMPONENTS casa)
    endif()
  endif()
  mark_as_advanced(PYRAP_INCLUDE_DIR PYRAP_LIBRARY)

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(Pyrap DEFAULT_MSG 
    PYRAP_LIBRARY PYRAP_INCLUDE_DIR)

  set(PYRAP_INCLUDE_DIRS ${PYRAP_INCLUDE_DIR} ${CASACORE_INCLUDE_DIRS})
  set(PYRAP_LIBRARIES ${PYRAP_LIBRARY} ${CASACORE_LIBRARIES})

endif()
