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

  message(STATUS "*** ENTER: lofar_find_package(${_package} ${ARGN})")

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

    # If package is required, but has been disabled explicitly, raise an
    # error.
    if(${_package}_FIND_REQUIRED AND DEFINED USE_${_PKG} AND NOT USE_${_PKG})
      message(SEND_ERROR 
        "Package ${_package} is required, but has been disabled explicitly!")
    endif(${_package}_FIND_REQUIRED AND DEFINED USE_${_PKG} AND NOT USE_${_PKG})

    # Use the Find${_package}.cmake module.
    message(STATUS "find_package(${ARGV})")
    find_package(${ARGV})

    # Define an all-uppercase variable for each mixed-case variable we're
    # interested in. This is a clumsy hack for modules like FindBoost.cmake
    foreach(_var INCLUDE_DIRS LIBRARIES DEFINITIONS ROOT_DIR FOUND)
      if(DEFINED ${_package}_${_var})
        set(${_PKG}_${_var} ${${_package}_${_var}})
      endif(DEFINED ${_package}_${_var})
    endforeach(_var INCLUDE_DIRS LIBRARIES DEFINITIONS ROOT_DIR FOUND)

    # Add include directories and libraries, if package was found.
    if(${_PKG}_FOUND)
      set(${_PKG}_FOUND ${${_PKG}_FOUND} PARENT_SCOPE)
      set(HAVE_${_PKG} TRUE CACHE BOOL "Have ${_package}")
      include_directories(${${_PKG}_INCLUDE_DIRS})
      ## Using link_libraries() would avoid the need to collect libs in
      ## LOFAR_LIBRARIES, but this command has been deprecated.
      # link_libraries(${${_PKG}_LIBRARIES}) 
      set(LOFAR_LIBRARIES ${LOFAR_LIBRARIES} ${${_PKG}_LIBRARIES} PARENT_SCOPE)
#      set_property(GLOBAL APPEND PROPERTY 
#        ${PROJECT_NAME}_LINK_LIBRARIES ${${_PKG}_LIBRARIES})
    endif(${_PKG}_FOUND)

  endif(NOT ${_PKG}_FOUND)

  message(STATUS "*** LEAVE: lofar_find_package(${_package} ${ARGN})")

endfunction(lofar_find_package _package)
