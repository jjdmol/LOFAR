# - Try to find PVSS (Prozessvisualisierungs- und Steuerungssystem)
# Variables used by this module:
#  PVSS_ROOT_DIR     - PVSS root directory
# Variables defined by this module:
#  PVSS_FOUND        - System has PVSS
#  PVSS_DEFINITIONS  - Compiler definitions required for PVSS 
#  PVSS_INCLUDE_DIR  - "Top-level" PVSS include directory (cached)
#  PVSS_INCLUDE_DIRS - List of PVSS include directories
#  PVSS_LIBRARY_DIR  - PVSS library directory (cached)
#  PVSS_LIBRARIES    - List of all PVSS libraries

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

# Compiler definitions required for PVSS
set(pvss_definitions
  -D__UNIX
  -D__PC
  -DHAS_TEMPLATES=1 
  -DBC_DESTRUCTOR 
  -Dbcm_boolean_h 
  -DOS_LINUX 
  -DLINUX 
  -DDLLEXP_BASICS= 
  -DDLLEXP_CONFIGS= 
  -DDLLEXP_DATAPOINT= 
  -DDLLEXP_MESSAGES= 
  -DDLLEXP_MANAGER= 
  -DDLLEXP_CTRL=)
  
# Define PVSS include directories, relative to top-level include directory.
set(pvss_include_dirs 
  Basics/DpBasics 
  Basics/Variables 
  Basics/Utilities 
  Basics/NoPosix 
  BCMNew 
  Configs 
  Datapoint 
  Messages 
  Manager)

# Define PVSS libraries.
set(pvss_libraries 
  Manager
  Messages 
  Datapoint 
  Basics 
  bcm)
  
if(NOT PVSS_FOUND)

  # Get the PVSS version information. This information can be found in the file
  # VersInfo.mk, which sets a number of Makefile variables. The version string
  # that is used in the filenames of the shared libraries can be found in the
  # variable PVSS_DLL_VERS. This variable is either set directly, or composed
  # from the Makefile variables PVSS_VERSION_MAIN and PVSS_VERSION_BUILD.
  find_file(PVSS_VERSINFO_MK
    NAMES VersInfo.mk
    HINTS ${PVSS_ROOT_DIR}
    PATH_SUFFIXES api)
  mark_as_advanced(PVSS_VERSINFO_MK)

  if(NOT PVSS_VERSINFO_MK)
    set(_errmsg "Could NOT find file api/VersInfo.mk.\nPlease make sure that PVSS_ROOT_DIR is set to the root directory of your PVSS installation.")
    if(PVSS_FIND_REQUIRED)
      message(FATAL_ERROR "${_errmsg}")
    elseif(NOT PVSS_FIND_QUIETLY)
      message(STATUS "${_errmsg}")
    endif()
    return()
  endif()

  # Get the main version
  file(STRINGS ${PVSS_VERSINFO_MK} match REGEX "^PVSS_VERSION_MAIN *=")
  string(REGEX REPLACE "^.*= *([^ ]+)$" "\\1" PVSS_VERSION_MAIN "${match}")

  # Get the build version
  file(STRINGS ${PVSS_VERSINFO_MK} match REGEX "^PVSS_VERSION_BUILD *=")
  string(REGEX REPLACE "^.*= *([^ ]+)$" "\\1" PVSS_VERSION_BUILD "${match}")

  # Get the library version (which may contains Makefile variables)
  file(STRINGS ${PVSS_VERSINFO_MK} match REGEX "^PVSS_DLL_VERS *=")
  string(REGEX REPLACE "^.*= *([^ ]+)$" "\\1" match "${match}")

  # Replace Makefile variables $(name) with CMake variables ${name}
  string(REGEX REPLACE "\\(([^)]+)\\)" "{\\1}" match "${match}")

  # Replace the CMake variables with their contents
  string(CONFIGURE "${match}" PVSS_DLL_VERS)

  message(STATUS "Searching for PVSS ${PVSS_DLL_VERS}")

  # Search for the PVSS include directory
  find_path(PVSS_INCLUDE_DIR
    NAMES Basics/Utilities/Util.hxx
    HINTS ${PVSS_ROOT_DIR}
    PATH_SUFFIXES api/include)
  set(PVSS_INCLUDE_DIRS ${PVSS_INCLUDE_DIR})
  set(pvss_check_list PVSS_INCLUDE_DIR)
  
  # Search for the PVSS libraries
  string(TOLOWER "${CMAKE_SYSTEM_NAME}" osname)
  foreach(lib ${pvss_libraries})
    find_library(PVSS_${lib}_LIBRARY
      NAMES ${lib}${PVSS_DLL_VERS}
      HINTS ${PVSS_ROOT_DIR}
      PATH_SUFFIXES api/lib.${osname} bin)
    list(APPEND pvss_check_list PVSS_${lib}_LIBRARY)
  endforeach()

  # Mark all variables in pvss_check_list as advanced
  mark_as_advanced(${pvss_check_list})
  
  # Handle the QUIETLY and REQUIRED arguments and set PVSS_FOUND to TRUE if
  # all elements of pvss_check_list are TRUE.
  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(PVSS DEFAULT_MSG ${pvss_check_list})
  
  # Now it's time to fill the non-cached variables
  if(PVSS_FOUND)
    foreach(def ${pvss_definitions})
      set(PVSS_DEFINITIONS "${PVSS_DEFINITIONS} ${def}")
    endforeach()
    foreach(dir ${pvss_include_dirs})
      list(APPEND PVSS_INCLUDE_DIRS ${PVSS_INCLUDE_DIR}/${dir})
    endforeach()
    foreach(lib ${pvss_libraries})
      list(APPEND PVSS_LIBRARIES ${PVSS_${lib}_LIBRARY})
    endforeach()
  endif()

endif()
