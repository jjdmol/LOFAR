# - A tiny wrapper around the FindPNG.cmake macro that comes with CMake. 
# Its purpose is to set variables that should have been set, according to the
# standard for FindXXX modules, but weren't.

# Copyright (C) 2010
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

# Call the "real" FindPNG module.
include(${CMAKE_ROOT}/Modules/FindPNG.cmake)

# Set PNG_INCLUDE_DIRS if PNG was found and PNG_INCLUDE_DIRS was not yet set.
if(PNG_FOUND)
  if(NOT PNG_INCLUDE_DIRS)
    set(PNG_INCLUDE_DIRS ${PNG_INCLUDE_DIR})
  endif(NOT PNG_INCLUDE_DIRS)
endif(PNG_FOUND)
