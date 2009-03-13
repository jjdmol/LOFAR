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

message(STATUS "**** ENTER: LofarFindPackage.cmake ****")

## ----------------------------------------------------------------------------
## Includes
## ----------------------------------------------------------------------------

# LOFAR CMake modules
include(LofarInit)
include(LofarSearchPath)

# Standard CMake modules
include(FindPackageHandleStandardArgs)

## ----------------------------------------------------------------------------
## function lofar_find_package
## ----------------------------------------------------------------------------
function(lofar_find_package _package)

  string(TOLOWER ${_package} _pkg)
  string(TOUPPER ${_package} _PKG)

  # Set CMAKE_PREFIX_PATH; used by the find_xxx() commands for searching.
  if(DEFINED ${_PKG}_PREFIX_PATH)
    set(CMAKE_PREFIX_PATH ${_PKG}_PREFIX_PATH)
  else(DEFINED ${_PKG}_PREFIX_PATH)
    lofar_search_path(_prefix_path ${_pkg})
    set(CMAKE_PREFIX_PATH ${_prefix_path})
  endif(DEFINED ${_PKG}_PREFIX_PATH)

  # Set the required and quiet variables for the package. This is not
  # necessary for find_package(), but we need to set them anyway if using the
  # generic find code.
  if(ARGV1)
    set(${_package}_FIND_REQUIRED 1)
  endif(ARGV1)
  if(NOT LOFAR_VERBOSE_CONFIGURE)
    set(${_package}_FIND_QUIETLY 1)
  endif(NOT LOFAR_VERBOSE_CONFIGURE)

  message(STATUS "${_package}_FIND_REQUIRED = ${${_package}_FIND_REQUIRED}")
  message(STATUS "USE_${_PKG} = ${USE_${_PKG}}")
  # If package is required, but has been disabled explicitly, raise an error.
  if(${_package}_FIND_REQUIRED AND DEFINED USE_${_PKG} AND NOT USE_${_PKG})
    message(SEND_ERROR 
      "Package ${_package} is required, but has been disabled explicitly!")
  endif(${_package}_FIND_REQUIRED AND DEFINED USE_${_PKG} AND NOT USE_${_PKG})

  if(${ARGC} LESS 3)
    # Use the Find${_package}.cmake module.
    find_package(${_package})

    # Define an all-uppercase variable for each mixed-case variable we're
    # interested in. This is a clumsy hack for modules like FindBoost.cmake
    foreach(_var INCLUDE_DIRS LIBRARIES DEFINITIONS ROOT_DIR FOUND)
      if(DEFINED ${_package}_${_var})
        set(${_PKG}_${_var} ${${_package}_${_var}})
      endif(DEFINED ${_package}_${_var})
    endforeach(_var INCLUDE_DIRS LIBRARIES DEFINITIONS ROOT_DIR FOUND)

 else(${ARGC} LESS 3)
    # Use generic find method.
    set(_headerfile ${ARGV2})
    if(DEFINED ARGV3)# AND NOT ARGV3 MATCHES "^$")
      set(_library ${ARGV3})
    else(DEFINED ARGV3)# AND NOT ARGV3 MATCHES "^$")
      set(_library ${_pkg})
    endif(DEFINED ARGV3)# AND NOT ARGV3 MATCHES "^$")

    # Search for the header file and set the package root directory.
    find_path(${_PKG}_INCLUDE_DIR ${_headerfile})
    get_filename_component(${_PKG}_ROOT_DIR ${${_PKG}_INCLUDE_DIR} PATH)

    message(STATUS "[1] ${_PKG}_ROOT_DIR = ${${_PKG}_ROOT_DIR}")

    # Search for the library in lib or lib64 (depending on the value of
    # FIND_LIBRARY_USE_LIB64_PATHS), relative to package root directory we
    # just found.
    if(NOT "${_library}" MATCHES "^$")
      find_library(${_PKG}_LIBRARY ${_library} ${${_PKG}_ROOT_DIR})
      if(NOT ${_PKG}_LIBRARY)
        set(${_PKG}_ROOT_DIR)
      endif(NOT ${_PKG}_LIBRARY)
    endif(NOT "${_library}" MATCHES "^$")

    message(STATUS "[2] ${_PKG}_ROOT_DIR = ${${_PKG}_ROOT_DIR}")

    # handle the QUIETLY and REQUIRED arguments and set ${_PKG}_FOUND to TRUE
    # if all listed variables are TRUE
    find_package_handle_standard_args(${_package} 
      "Could NOT find ${_package} in ${CMAKE_PREFIX_PATH}" ${_PKG}_ROOT_DIR)
#      ${_PKG}_LIBRARY ${_PKG}_INCLUDE_DIR)

  endif(${ARGC} LESS 3)

  message(STATUS "${_PKG}_INCLUDE_DIR = ${${_PKG}_INCLUDE_DIR}")
  message(STATUS "${_PKG}_LIBRARY = ${${_PKG}_LIBRARY}")
  message(STATUS "${_PKG}_FOUND = ${${_PKG}_FOUND}")
  message(STATUS "HAVE_${_PKG} = ${HAVE_${_PKG}}")

  if(${_PKG}_FOUND AND NOT HAVE_${_PKG})
    MESSAGE(STATUS "Setting LOFAR_ vars...")
    set(HAVE_${_PKG} TRUE PARENT_SCOPE)
    list(APPEND LOFAR_INCLUDE_DIRS ${${_PKG}_INCLUDE_DIR})
    list(APPEND LOFAR_LIBRARIES ${${_PKG}_LIBRARY})
    list(APPEND LOFAR_DEFINITIONS HAVE_${_PKG})
    set(LOFAR_INCLUDE_DIRS ${LOFAR_INCLUDE_DIRS} PARENT_SCOPE)
    set(LOFAR_LIBRARIES ${LOFAR_LIBRARIES} PARENT_SCOPE)
    set(LOFAR_DEFINITIONS ${LOFAR_DEFINITIONS} PARENT_SCOPE)
#    set(LOFAR_INCLUDE_DIRS "${LOFAR_INCLUDE_DIRS} -I${${_PKG}_INCLUDE_DIR}" PARENT_SCOPE)
#    set(LOFAR_LIBRARIES "${LOFAR_LIBRARIES} ${${_PKG}_LIBRARY}" PARENT_SCOPE)
#    set(LOFAR_DEFINITIONS "${LOFAR_DEFINITIONS} -DHAVE_${_PKG}" PARENT_SCOPE)

#    include_directories(${${_PKG}_INCLUDE_DIR})
#    add_definitions(-DHAVE_${_PKG})

#    set_property(GLOBAL APPEND PROPERTY LOFAR_INCLUDE_DIRS ${${_PKG}_INCLUDE_DIR})
#    set_property(GLOBAL APPEND PROPERTY LOFAR_LIBRARIES ${${_PKG}_LIBRARY})
#    set_property(GLOBAL APPEND PROPERTY LOFAR_DEFINITIONS HAVE_${_PKG})
  endif(${_PKG}_FOUND AND NOT HAVE_${_PKG})

endfunction(lofar_find_package _package)

message(STATUS "**** LEAVE: LofarFindPackage.cmake ****")
