# - Try to find Casa include dirs and libraries
# Usage:
#   find_package(Casa [REQUIRED] [COMPONENTS components...])
# Valid components are:
#   msvis, calibration, synthesis, flagging, simulators
#
# Note that most components are dependent on other (more basic) components.
# In that case, it suffices to specify the "top-level" components; dependent
# components will be searched for automatically.
#
# Variables used by this module:
#  CASA_ROOT_DIR         - Casa root directory. 
#
# Variables defined by this module:
#  CASA_FOUND            - System has Casa, which means that the
#                              include dir was found, as well as all 
#                              libraries specified (not cached)
#  CASA_INCLUDE_DIR      - Casa include directory (cached)
#  CASA_INCLUDE_DIRS     - Casa include directories (not cached)
#                              identical to CASA_INCLUDE_DIR
#  CASA_LIBRARIES        - The Casa libraries (not cached)
#  CASA_${COMPONENT}_LIBRARY - The absolute path of Casa library 
#                              "component" (cached)
#  HAVE_CASA             - True if system has Casa (cached)
#                              identical to CASA_FOUND
#
# ATTENTION: The component names need to be in lower case, just as the
# casa library names. However, the CMake variables use all upper case.

# Copyright (C) 2011
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
# $Id: FindCasa.cmake 15228 2010-03-16 09:27:26Z loose $

# - casa_resolve_dependencies(_result)
#
# Resolve the Casa library dependencies for the given components. 
# The list of dependent libraries will be returned in the variable result.
# It is sorted from least dependent to most dependent library, so it can be
# directly fed to the linker.
#
#   Usage: casa_resolve_dependencies(result components...)
#
macro(casa_resolve_dependencies _result)
  set(${_result} ${ARGN})
  set(_index 0)
  # Do a breadth-first search through the dependency graph; append to the
  # result list the dependent components for each item in that list. 
  # Duplicates will be removed later.
  while(1)
    list(LENGTH ${_result} _length)
    if(NOT _index LESS _length)
      break()
    endif(NOT _index LESS _length)
    list(GET ${_result} ${_index} item)
    list(APPEND ${_result} ${Casa_${item}_DEPENDENCIES})
    math(EXPR _index "${_index}+1")
  endwhile(1)
  # Remove all duplicates in the current result list, while retaining only the
  # last of each duplicate.
  list(REVERSE ${_result})
  list(REMOVE_DUPLICATES ${_result})
  list(REVERSE ${_result})
endmacro(casa_resolve_dependencies _result)


# - casa_find_library(_name)
#
# Search for the library ${_name}. 
# If library is found, add it to CASA_LIBRARIES; if not, add ${_name}
# to CASA_MISSING_COMPONENTS and set CASA_FOUND to false.
#
#   Usage: casa_find_library(name)
#
macro(casa_find_library _name)
  string(TOUPPER ${_name} _NAME)
  find_library(${_NAME}_LIBRARY ${_name}
    HINTS ${CASA_ROOT_DIR} PATH_SUFFIXES lib)
  mark_as_advanced(${_NAME}_LIBRARY)
  if(${_NAME}_LIBRARY)
    list(APPEND CASA_LIBRARIES ${${_NAME}_LIBRARY})
  else(${_NAME}_LIBRARY)
    set(CASA_FOUND FALSE)
    list(APPEND CASA_MISSING_COMPONENTS ${_name})
  endif(${_NAME}_LIBRARY)
endmacro(casa_find_library _name)


# - casa_find_package(_name)
#
# Search for the package ${_name}.
# If the package is found, add the contents of ${_name}_INCLUDE_DIRS to
# CASA_INCLUDE_DIRS and ${_name}_LIBRARIES to CASA_LIBRARIES.
#
# If Casa itself is required, then, strictly speaking, the packages it
# requires must be present. However, when linking against static libraries
# they may not be needed. One can override the REQUIRED setting by switching
# CASA_MAKE_REQUIRED_EXTERNALS_OPTIONAL to ON. Beware that this might cause
# compile and/or link errors.
#
#   Usage: casa_find_package(name [REQUIRED])
#
macro(casa_find_package _name)
  if("${ARGN}" MATCHES "^REQUIRED$" AND
      Casa_FIND_REQUIRED AND
      NOT CASA_MAKE_REQUIRED_EXTERNALS_OPTIONAL)
    find_package(${_name} REQUIRED)
  else()
    find_package(${_name})
  endif()
  if(${_name}_FOUND)
    list(APPEND CASA_INCLUDE_DIRS ${${_name}_INCLUDE_DIRS})
    list(APPEND CASA_LIBRARIES ${${_name}_LIBRARIES})
  endif(${_name}_FOUND)
endmacro(casa_find_package _name)


# Define the Casa components.
set(Casa_components
  synthesis
)

# Define the Casa components' inter-dependencies.
# set(Casa_calibration_DEPENDENCIES  msvis)
# set(Casa_synthesis_DEPENDENCIES calibration)
# set(Casa_flagging_DEPENDENCIES  msvis)

# Initialize variables.
set(CASA_FOUND FALSE)
set(CASA_DEFINITIONS)
set(CASA_LIBRARIES)
set(CASA_MISSING_COMPONENTS)

# Search for the header file first. Note that casa installs the header
# files in ${prefix}/include/casacode, instead of ${prefix}/include.
if(NOT CASA_INCLUDE_DIR)
  find_path(CASA_INCLUDE_DIR synthesis/MSVis/VisSet.h
    HINTS ${CASA_ROOT_DIR} PATH_SUFFIXES include/casacode)
  mark_as_advanced(CASA_INCLUDE_DIR)
endif(NOT CASA_INCLUDE_DIR)

if(NOT CASA_INCLUDE_DIR)
  set(CASA_ERROR_MESSAGE "Casa: unable to find the header file synthesis/MSVis/VisSet.h.\nPlease set CASA_ROOT_DIR to the root directory containing Casa.")
else(NOT CASA_INCLUDE_DIR)
  # We've found the header file; let's continue.
  set(CASA_FOUND TRUE)
  set(CASA_INCLUDE_DIRS ${CASA_INCLUDE_DIR})

  # If the user specified components explicity, use that list; otherwise we'll
  # assume that the user wants to use all components.
  if(NOT Casa_FIND_COMPONENTS)
    set(Casa_FIND_COMPONENTS ${Casa_components})
  endif(NOT Casa_FIND_COMPONENTS)

  # Get a list of all dependent Casa libraries that need to be found.
  casa_resolve_dependencies(_find_components ${Casa_FIND_COMPONENTS})

  # Find the library for each component, and handle external dependencies
  foreach(_comp ${_find_components})
    casa_find_library(${_comp})
  endforeach(_comp ${_find_components})

endif(NOT CASA_INCLUDE_DIR)

# Set HAVE_CASA
if(CASA_FOUND)
  set(HAVE_CASA TRUE CACHE INTERNAL "Define if Casa is installed")
endif(CASA_FOUND)

# Compose diagnostic message if not all necessary components were found.
if(CASA_MISSING_COMPONENTS)
  set(CASA_ERROR_MESSAGE "Casa: the following components could not be found:\n     ${CASA_MISSING_COMPONENTS}")
endif(CASA_MISSING_COMPONENTS)

# Print diagnostics.
if(CASA_FOUND)
  if(NOT Casa_FIND_QUIETLY)
    message(STATUS "Found the following Casa components: ")
    foreach(comp ${_find_components})
      message(STATUS "  ${comp}")
    endforeach(comp ${_find_components})
  endif(NOT Casa_FIND_QUIETLY)
else(CASA_FOUND)
  if(Casa_FIND_REQUIRED)
    message(FATAL_ERROR "${CASA_ERROR_MESSAGE}")
  else(Casa_FIND_REQUIRED)
    message(STATUS "${CASA_ERROR_MESSAGE}")
  endif(Casa_FIND_REQUIRED)
endif(CASA_FOUND)
