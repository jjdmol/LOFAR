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

include(LofarSearchPath)
include(FindPackageHandleStandardArgs)

# ----------------------------------------------------------------------------
# function lofar_find_package(package)
#
# Find a package like find_package() does.
# Use the LOFAR search path to locate the package, unless the package's root
# directory <PKG>_ROOT_DIR is defined.
#
# Furthermore:
# - Add preprocessor definitions that are defined in <PKG>_DEFINITIONS.
# - Add include directories that are defined <PKG>_INCLUDE_DIRS.
# - Add <PKG>_LIBRARIES to the list of LOFAR_EXTRA_LIBRARIES, needed for
#   linking.
# - Add cache variable HAVE_<PKG>, which indicates whether the package was 
#   found. It can be used with #cmakedefine.
# Note: <PKG> equals <package> in uppercase.
# ----------------------------------------------------------------------------
function(lofar_find_package _package)

  string(TOLOWER ${_package} _pkg)
  string(TOUPPER ${_package} _PKG)

  if(NOT ${_PKG}_FOUND)

    # Set CMAKE_PREFIX_PATH, used by the find_xxx() commands, to the package's
    # root directory ${_PKG}_ROOT_DIR, if defined; otherwise set it to the
    # LOFAR search path.
    if(${_PKG}_ROOT_DIR)
      set(CMAKE_PREFIX_PATH ${${_PKG}_ROOT_DIR})
    else(${_PKG}_ROOT_DIR)
      lofar_search_path(CMAKE_PREFIX_PATH ${_pkg})
    endif(${_PKG}_ROOT_DIR)

    # Search the package using the Find${_package}.cmake module, unless the
    # package have been disabled explicitly.
    if(NOT DEFINED USE_${_PKG} OR USE_${_PKG})
      if(LOFAR_VERBOSE_CONFIGURE)
        find_package(${ARGV})
      else(LOFAR_VERBOSE_CONFIGURE)
        find_package(${ARGV} QUIET)
      endif(LOFAR_VERBOSE_CONFIGURE)
    else(NOT DEFINED USE_${_PKG} OR USE_${_PKG})
      list(FIND ARGN REQUIRED is_required)
      if(is_required GREATER -1)
        message(SEND_ERROR 
          "Package ${_package} is required, but has been disabled explicitly!")
      else(is_required GREATER -1)
        message(STATUS "Package ${_package} has been disabled explicitly")
      endif(is_required GREATER -1)
    endif(NOT DEFINED USE_${_PKG} OR USE_${_PKG})

    # Add include directories and libraries, if package was found;
    # set HAVE_<PACKAGE> variable in the cache.
    if(${_PKG}_FOUND)
      if(NOT DEFINED HAVE_${_PKG})
        set(HAVE_${_PKG} TRUE CACHE INTERNAL "Have ${_package}?")
      endif(NOT DEFINED HAVE_${_PKG})
      add_definitions(${${_PKG}_DEFINITIONS})
      include_directories(${${_PKG}_INCLUDE_DIRS})
      set(LOFAR_EXTRA_LIBRARIES ${LOFAR_EXTRA_LIBRARIES} ${${_PKG}_LIBRARIES}
        PARENT_SCOPE)
    else(${_PKG}_FOUND)
      set(HAVE_${_PKG} FALSE CACHE INTERNAL "Have ${_package}?")
    endif(${_PKG}_FOUND)
    set(${_PKG}_FOUND ${${_PKG}_FOUND} PARENT_SCOPE)

  endif(NOT ${_PKG}_FOUND)

endfunction(lofar_find_package _package)
