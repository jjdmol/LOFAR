# - Try to find Blitz: a C++ template class library for scientific computing
# This will define:
#  BLITZ_FOUND        - system has Blitz
#  BLITZ_INCLUDE_DIR  - the Blitz include directory (cached)
#  BLITZ_INCLUDE_DIRS - the Blitz include directories 
#                       (identical to BLITZ_INCLUDE_DIR)
#  BLITZ_LIBRARY      - the Blitz library (cached)
#  BLITZ_LIBRARIES    - the Blitz libraries
#                       (identical to BLITZ_LIBRARY)

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

if(NOT BLITZ_FOUND)

  find_path(BLITZ_INCLUDE_DIR blitz/blitz.h)
  find_library(BLITZ_LIBRARY blitz)
  mark_as_advanced(BLITZ_INCLUDE_DIR BLITZ_LIBRARY)
  
  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(Blitz DEFAULT_MSG
    BLITZ_LIBRARY BLITZ_INCLUDE_DIR)

  set(BLITZ_INCLUDE_DIRS ${BLITZ_INCLUDE_DIR})
  set(BLITZ_LIBRARIES ${BLITZ_LIBRARY})

endif(NOT BLITZ_FOUND)
