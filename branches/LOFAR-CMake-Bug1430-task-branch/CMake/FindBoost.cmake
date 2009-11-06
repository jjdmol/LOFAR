# $Id$
#
# Copyright (C) 2008-2009
# ASTRON (Netherlands Foundation for Research in Astronomy)
# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

# A tiny wrapper around the FindBoost.cmake macro that comes with CMake. 
# Its purpose is threefold:
# - Remove Boost components that have been disabled explicitly from the
#   Boost_FIND_COMPONENTS list. Raise an error, if the component is required.
# - Define all-uppercase variables for the following variables: 
#   Boost_INCLUDE_DIRS, Boost_LIBRARIES, and Boost_FOUND.
# - Set a HAVE_BOOST_<COMPONENT> variable in the cache for each component that
#   was found.

# Boost components that have been disabled explicitly by the user, should be
# removed from the Boost_FIND_COMPONENTS list.
foreach(_comp ${Boost_FIND_COMPONENTS})
  string(TOUPPER "${_comp}" _COMP)
  if(DEFINED USE_BOOST_${_COMP} AND NOT USE_BOOST_${_COMP})
    # Remove disabled item from the component list.
    list(REMOVE_ITEM Boost_FIND_COMPONENTS ${_comp})
    if(Boost_FIND_REQUIRED)
      message(SEND_ERROR "Boost component ${_comp} is required, "
        "but has been disabled explicitly!")
    else(Boost_FIND_REQUIRED)
      message(STATUS "Boost component ${_comp} has been disabled explicitly")
    endif(Boost_FIND_REQUIRED)
  endif(DEFINED USE_BOOST_${_COMP} AND NOT USE_BOOST_${_COMP})
endforeach(_comp ${Boost_FIND_COMPONENTS})

# Call the "real" FindBoost module.
include(${CMAKE_ROOT}/Modules/FindBoost.cmake)

# Define all-uppercase variables for Boost_INCLUDE_DIRS Boost_LIBRARIES and 
# Boost_FOUND.
foreach(_var INCLUDE_DIRS LIBRARIES FOUND)
  if(DEFINED Boost_${_var})
    set(BOOST_${_var} ${Boost_${_var}})
  endif(DEFINED Boost_${_var})
endforeach(_var INCLUDE_DIRS LIBRARIES FOUND)

# Define a HAVE_BOOST_<COMPONENT> variable, for each component that was found.
foreach(_comp ${Boost_FIND_COMPONENTS})
  string(TOUPPER "${_comp}" _comp)
  if(Boost_${_comp}_FOUND)
    set(HAVE_BOOST_${_comp} TRUE CACHE INTERNAL 
      "Set if Boost component ${_comp} was found")
  endif(Boost_${_comp}_FOUND)
endforeach(_comp ${Boost_FIND_COMPONENTS})
