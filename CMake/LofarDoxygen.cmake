# - Generate script to configure and run the code documentation tool Doxygen.

# $Id$
#
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

include(LofarMacros)

if(USE_DOXYGEN)
  # Locate the doxygen programs.
  find_package(Doxygen REQUIRED)

  # Document all source code, unless BUILD_PACKAGES is defined.
  if(NOT DEFINED BUILD_PACKAGES)
    set(DOXYGEN_INPUT ${CMAKE_SOURCE_DIR})
  else(NOT DEFINED BUILD_PACKAGES)
    # We need the list of package source directories.
    include(LofarPackageList)
    set(DOXYGEN_INPUT)
    foreach(_pkg ${BUILD_PACKAGES})
      list(APPEND DOXYGEN_INPUT ${${_pkg}_SOURCE_DIR})
    endforeach(_pkg ${BUILD_PACKAGES})
    lofar_join_arguments(DOXYGEN_INPUT)
  endif(NOT DEFINED BUILD_PACKAGES)

  # Full path to the generated Doxygen configuration file.
  set(DOXYGEN_CONFIG_FILE "${CMAKE_BINARY_DIR}/doxygen.cfg")

  # Define custom target 'doc'. The reason for using an external script
  # MakeDoxyDoc.cmake, instead of defining the custom target directly, is that
  # we want to be able to redirect stdout and stderr, which can only be done
  # using execute_process().
  add_custom_target(doc
      COMMAND "${CMAKE_COMMAND}" -P "${CMAKE_BINARY_DIR}/MakeDoxyDoc.cmake"
      COMMENT "Generating code documentation. Please be patient ...")

  # Generate the CMake script that will be invoked by 'make doc'.
  # The reason for using an external script, instead of defining the custom
  # target directly, is that we want to be able to redirect stdout and stderr.
  configure_file(
    "${CMAKE_SOURCE_DIR}/CMake/docscripts/MakeDoxyDoc.cmake.in"
    "${CMAKE_BINARY_DIR}/MakeDoxyDoc.cmake" @ONLY)

  # Generate the Doxygen configuration file, used by Doxygen.
  configure_file(
    "${CMAKE_SOURCE_DIR}/CMake/docscripts/doxygen.cfg.in"
    "${DOXYGEN_CONFIG_FILE}" @ONLY)

endif(USE_DOXYGEN)
