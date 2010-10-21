# - Build a shared library from one or more Matlab M-files
# This module defines the macro:
#  lofar_add_matlab_library(<name> <m-file> [<m-file> ...])
# which compiles one or more M-files into a shared library lib<name>.so,
# and a header file lib<name>.h that defines the interface to lib<name>.so.
# Furthermore, the generated library is added to the list of libraries for 
# the current package (global property ${PACKAGE_NAME}_LIBRARIES).

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

# Check that the Matlab to C/C++ Compiler is available.
include(LofarFindPackage)
lofar_find_package(Matlab)

macro(lofar_add_matlab_library _name)

  # MCC, the Matlab to C/C++ Compiler, must be present.
  if(NOT MATLAB_COMPILER)
    message(FATAL_ERROR "Could not find MCC - the Matlab to C/C++ Compiler")
  endif(NOT MATLAB_COMPILER)

  # The names of the generated files must all be prefixed with "lib"
  set(_libname "lib${_name}")

  # Determine the destination directory for the generated C++ header file.
  # This directory is chosen such that other MAC packages only have to
  # add <binary-root>/include/MAC to their -I path.
  if(DEFINED ${PACKAGE_NAME}_INCLUDE_PATH_SUFFIX)
    set(_incdir
      "${CMAKE_BINARY_DIR}/include/MAC/${${PACKAGE_NAME}_INCLUDE_PATH_SUFFIX}")
  else()
    string(REGEX REPLACE "${LOFAR_ROOT}" "${CMAKE_BINARY_DIR}/include"
      _incdir "${${PACKAGE_NAME}_SOURCE_DIR}")
  endif(DEFINED ${PACKAGE_NAME}_INCLUDE_PATH_SUFFIX)

  # If we're doing a debug build, then also pass debug flags to MCC
  if(LOFAR_BUILD_VARIANT MATCHES "^DEBUG$")
    set(MATLAB_COMPILER_OPTIONS "-g")
  endif(LOFAR_BUILD_VARIANT MATCHES "^DEBUG$")

  # List of generated files
  set(_generated_files 
    ${_libname}.h ${_libname}.cpp ${_libname}.so "${_incdir}/${_name}.h")

  # Define the command to generate a C++ header and source file, and a shared
  # library, from one or more Matlab M-files. The C++ header file will be
  # copied to the correct destination directory.
  add_custom_command(OUTPUT ${_generated_files}
    COMMAND "${MATLAB_COMPILER}" ${MATLAB_COMPILER_OPTIONS}
      -I "${CMAKE_CURRENT_SOURCE_DIR}" -B cpplib:${_libname} ${ARGN}
    COMMAND "${CMAKE_COMMAND}"
      -E copy "${_libname}.h" "${_incdir}/${_name}.h"
    DEPENDS ${ARGN}
    COMMENT "[MCC][${_name}] Building CXX shared library from ${ARGN}")

  # Add a custom target <name> to trigger the generation of the C++ files
  add_custom_target(${_name} ALL DEPENDS ${_generated_files})

  # Add a dependency of the current package on the target <name>
  add_dependencies(${PACKAGE_NAME} ${_name})

  # Add the generated library to the list of libraries for the current package.
  set_property(GLOBAL APPEND PROPERTY 
    ${PACKAGE_NAME}_LIBRARIES ${CMAKE_CURRENT_BINARY_DIR}/${_libname}.so)

  # Install the generated shared library.
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${_libname}.so
    DESTINATION ${LOFAR_LIBDIR})

endmacro(lofar_add_matlab_library)
