# - LOFAR package related macros.
#
# The following macros/functions are defined:
#  lofar_add_package(pkg [srcdir] [REQUIRED])    {function}
#  lofar_package(<name> [version] [DEPENDS <depend> [depend] ...])    {macro}
#
# lofar_add_package() adds a LOFAR package to the build, unless it was
# excluded from the build (i.e., option BUILD_<pkg> is OFF). 
#
# Adding a package implies:
#  - If the target <pkg> is not yet defined:
#    - If LOFAR_SVN_UPDATE is ON, or if source directory does not yet exist:
#      - Do an 'svn update'
#    - If the package source directory exists:
#      - Define the option BUILD_<pkg>
#      - Set the variables PACKAGE_SOURCE_DIR and <pkg>_SOURCE_DIR to the
#        source directory of <pkg>, and the variables PACKAGE_BINARY_DIR and
#        <pkg>_BINARY_DIR to the binary directory of <pkg>
#      - Add a custom target <pkg>
#      - Add the source directory to the build
#    - Else: raise an error
#  - If the target <pkg> is defined:
#    - Add a dependency of the current package on package <pkg>
# Furthermore:
#  - If [srcdir] is not supplied, <pkg>_SOURCE_DIR, which must be defined in
#    that case, is used as directory name;
#  - It is not an error if the package source directory does not exist,
#    unless the REQUIRED keyword is specified.
# Note:
#   lofar_add_package() is intentionally declared as a function, to keep the
#   scope of PACKAGE_NAME local. This way, we can keep track of the name of
#   our "parent" package when lofar_add_package() is called recursively.
#
# lofar_package() defines a LOFAR package.
#
# This macro sets the following variables:
#   ${pkg}_VERSION        Version number of package <pkg>
#   PACKAGE_VERSION       (idem)
#   ${pkg}_DEPENDENCIES   List of packages that package <pkg> depends on.
#   PACKAGE_DEPENDENCIES  (idem)
# Each dependent package is added to the build. If any of these packages is
# excluded from the build (e.g., because BUILD_<dep> is OFF), then package
# <pkg> will also be excluded from the build.
#
# The include directories of each dependent package will be added to the
# include directories of package <pkg>; this is needed, because CMake does
# not retain the include path across source directories. The libraries
# created by each dependent package will be added to the list of link 
# libraries of package <pkg>.
#
# A preprocessor definition for LOFARLOGGER_PACKAGE is added.

#  Copyright (C) 2008-2010
#  ASTRON (Netherlands Foundation for Research in Astronomy)
#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, softwaresupport@astron.nl
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


if(NOT LOFAR_PACKAGE_INCLUDED)

  set(LOFAR_PACKAGE_INCLUDED TRUE)

  # Include LOFAR package list
  include(LofarPackageList)

  # Include LOFAR Subversion macros
  include(LofarSubversion)

  # Create custom target for top-level project (i.e. LOFAR)
  add_custom_target(${CMAKE_PROJECT_NAME})

  # Initialize PACKAGE_NAME to the name of the top-level project
  set(PACKAGE_NAME ${CMAKE_PROJECT_NAME})

  ############################################################################

  function(lofar_add_package _pkg)
    if(NOT DEFINED BUILD_${_pkg} OR BUILD_${_pkg})
      add_dependencies(${PACKAGE_NAME} ${_pkg})
      set(PACKAGE_NAME ${_pkg})
      if(NOT TARGET ${_pkg})
        string(REGEX REPLACE ";?REQUIRED$" "" _srcdir "${ARGN}")
        string(REGEX MATCH "REQUIRED$" _required "${ARGN}")
        if(_srcdir MATCHES "^$")
          if(NOT DEFINED ${_pkg}_SOURCE_DIR)
            message(FATAL_ERROR "Variable ${_pkg}_SOURCE_DIR is undefined!\n"
              "Please regenerate LofarPackageList.cmake.\n")
          endif(NOT DEFINED ${_pkg}_SOURCE_DIR)
          set(_srcdir ${${_pkg}_SOURCE_DIR})
        endif(_srcdir MATCHES "^$")
        if(NOT IS_ABSOLUTE ${_srcdir})
          get_filename_component(_srcdir ${_srcdir} ABSOLUTE)
        endif(NOT IS_ABSOLUTE ${_srcdir})
        string(REGEX REPLACE
          ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} _bindir ${_srcdir})
        message(STATUS "Adding package ${_pkg} ...")
        if(NOT DEFINED LOFAR_SVN_UPDATE OR LOFAR_SVN_UPDATE)
          if(LOFAR_SVN_UPDATE OR NOT EXISTS ${_srcdir})
            message(STATUS "  Updating ${_srcdir} ...")
            lofar_subversion_update(${_srcdir})
          endif(LOFAR_SVN_UPDATE OR NOT EXISTS ${_srcdir})
        endif(NOT DEFINED LOFAR_SVN_UPDATE OR LOFAR_SVN_UPDATE)
        if(EXISTS ${_srcdir})
          option(BUILD_${_pkg} "Build package ${_pkg}?" ON)
          mark_as_advanced(BUILD_${_pkg})
          set(PACKAGE_SOURCE_DIR ${_srcdir})
          set(PACKAGE_BINARY_DIR ${_bindir})
          set(${_pkg}_SOURCE_DIR ${_srcdir})
          set(${_pkg}_BINARY_DIR ${_bindir})
          add_custom_target(${_pkg})
          add_subdirectory(${_srcdir} ${_bindir})
        else(EXISTS ${_srcdir})
          set(_errmsg "Source package `${_pkg}' not found!"
            "  (directory ${_srcdir} does not exist)")
          if(_required)
            message(FATAL_ERROR ${_errmsg})
          else(_required)
            message(STATUS ${_errmsg})
          endif(_required)
        endif(EXISTS ${_srcdir})
      endif(NOT TARGET ${_pkg})
    endif(NOT DEFINED BUILD_${_pkg} OR BUILD_${_pkg})
  endfunction(lofar_add_package _pkg)

  ############################################################################
  macro(lofar_package _pkg)

    set(_errmsg
      "Wrong arguments supplied to lofar_package().\n"
      "Usage: lofar_package(pkg [version] [DEPENDS depend ...])\n")

    # Define a target for the current package, if it is not defined yet.
    if(NOT TARGET ${_pkg})    
      add_custom_target(${_pkg})
    endif(NOT TARGET ${_pkg})

    # Get the optional version number; a string of dot-separated numbers
    string(REGEX REPLACE ";?DEPENDS.*" "" ${_pkg}_VERSION "${ARGN}")
    if(NOT ${_pkg}_VERSION MATCHES "^([0-9]+(\\.[0-9]+)*)?$")
      message(FATAL_ERROR ${_errmsg})
    endif(NOT ${_pkg}_VERSION MATCHES "^([0-9]+(\\.[0-9]+)*)?$")

    # Get the optional package dependencies
    string(REGEX MATCH "DEPENDS;?.*" _depends "${ARGN}")
    if(_depends MATCHES "^.+$")
      string(REGEX REPLACE "DEPENDS;?" "" ${_pkg}_DEPENDENCIES "${_depends}")
      if(${_pkg}_DEPENDENCIES MATCHES "^$")
        message(FATAL_ERROR ${_errmsg})
      endif(${_pkg}_DEPENDENCIES MATCHES "^$")
    endif(_depends MATCHES "^.+$")

    set(PACKAGE_VERSION ${${_pkg}_VERSION})
    set(PACKAGE_DEPENDENCIES ${${_pkg}_DEPENDENCIES})

    if(LOFAR_VERBOSE_CONFIGURE)
      message(STATUS "  ${_pkg} version: ${${_pkg}_VERSION}")
      message(STATUS "  ${_pkg} dependencies: ${${_pkg}_DEPENDENCIES}")
    endif(LOFAR_VERBOSE_CONFIGURE)

    # Add all packages that <pkg> depends on to the build. If any of these
    # packages are excluded from the build (because BUILD_<dep> is OFF), then
    # BUILD_<pkg> will also be set to OFF and an error is raised.
    foreach(_dep ${${_pkg}_DEPENDENCIES})

      # Break out of foreach-loop if building of package <pkg> is disabled.
      if(DEFINED BUILD_${_pkg} AND NOT BUILD_${_pkg})
        break()
      endif(DEFINED BUILD_${_pkg} AND NOT BUILD_${_pkg})

      # Add package <dep> to the build. Don't worry about multiple inclusion;
      # lofar_add_package() will guard against it.
      lofar_add_package(${_dep})# REQUIRED)

      # If building of dependent package <dep> is enabled:
      #   add list of include directories of <dep> to that of package <pkg>
      #   add libraries created by <dep> to list of link libraries of <pkg>
      # else: 
      #   disable building of package <pkg> as well and raise an error.
      if(BUILD_${_dep})
        get_directory_property(_dirs
          DIRECTORY ${${_dep}_SOURCE_DIR} INCLUDE_DIRECTORIES)
        # Remove duplicate entries which may be present since
        # include_directories in cmake >= 2.8.8 does not remove them.
        list(REMOVE_DUPLICATES _dirs)
        include_directories(${_dirs})
        get_property(_libs GLOBAL PROPERTY ${_dep}_LIBRARIES)
        list(APPEND ${_pkg}_LINK_LIBRARIES ${_libs})
      else(BUILD_${_dep})
        set(BUILD_${_pkg} OFF CACHE BOOL "Build package ${_pkg}" FORCE)
        message(SEND_ERROR "Package `${_dep}' is excluded from the build, but "
          "`${_pkg}' depends on it. Package `${_pkg}' will be excluded as well!")
      endif(BUILD_${_dep})

    endforeach(_dep ${${_pkg}_DEPENDENCIES})

    # Add a preprocessor definition for the LOFAR Logger package
    string(REGEX REPLACE "^${LOFAR_ROOT}" "" _lpkg "${${_pkg}_SOURCE_DIR}")
    string(REGEX REPLACE "^/" "" _lpkg "${_lpkg}")
    string(REPLACE "/" "." _lpkg "${_lpkg}")
    add_definitions(-DLOFARLOGGER_PACKAGE="${_lpkg}")

  endmacro(lofar_package)

endif(NOT LOFAR_PACKAGE_INCLUDED)
