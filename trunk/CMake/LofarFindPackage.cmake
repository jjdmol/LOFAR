#  LofarFindPackage.cmake: 
#
#  Copyright (C) 2008-2009
#  ASTRON (Netherlands Foundation for Research in Astronomy)
#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
#  $Id$

## ----------------------------------------------------------------------------
## Includes
## ----------------------------------------------------------------------------
include(LofarSearchPath)
include(FindPackageHandleStandardArgs)

## ----------------------------------------------------------------------------
## function lofar_find_package
## ----------------------------------------------------------------------------
function(lofar_find_package _package)

  string(TOLOWER ${_package} _pkg)
  string(TOUPPER ${_package} _PKG)

  if(NOT ${_PKG}_FOUND)

    # Set CMAKE_PREFIX_PATH; used by the find_xxx() commands for searching.
    if(DEFINED ${_PKG}_PREFIX_PATH)
      set(CMAKE_PREFIX_PATH ${${_PKG}_PREFIX_PATH})
    else(DEFINED ${_PKG}_PREFIX_PATH)
      lofar_search_path(_prefix_path ${_pkg})
      set(CMAKE_PREFIX_PATH ${_prefix_path})
    endif(DEFINED ${_PKG}_PREFIX_PATH)

    # If package has been disabled explicitly, but is required, raise an
    # error.
    if(DEFINED USE_${_PKG} AND NOT USE_${_PKG})
      list(FIND ARGN REQUIRED is_required)
      if(is_required GREATER -1)
        message(SEND_ERROR 
          "Package ${_package} is required, but has been disabled explicitly!")
      endif(is_required GREATER -1)
    endif(DEFINED USE_${_PKG} AND NOT USE_${_PKG})

    # Use the Find${_package}.cmake module.
    find_package(${ARGV})

    # Add include directories and libraries, if package was found;
    # set HAVE_<PACKAGE> variable in the cache.
    if(${_PKG}_FOUND)
      if(NOT DEFINED HAVE_${_PKG})
        set(HAVE_${_PKG} TRUE CACHE INTERNAL "Have ${_package}?")
      endif(NOT DEFINED HAVE_${_PKG})
      add_definitions(${${_PKG}_DEFINITIONS})
      include_directories(${${_PKG}_INCLUDE_DIRS})
      set(LOFAR_LIBRARIES ${LOFAR_LIBRARIES} ${${_PKG}_LIBRARIES} PARENT_SCOPE)
    else(${_PKG}_FOUND)
      set(HAVE_${_PKG} FALSE CACHE INTERNAL "Have ${_package}?")
    endif(${_PKG}_FOUND)
    set(${_PKG}_FOUND ${${_PKG}_FOUND} PARENT_SCOPE)

  endif(NOT ${_PKG}_FOUND)

endfunction(lofar_find_package _package)
