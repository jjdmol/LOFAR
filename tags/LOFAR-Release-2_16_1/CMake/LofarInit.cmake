# - Initialize the LOFAR CMake build environment. 
#
# LofarInit.cmake must be included before the project() command.
#
# The following variables and properties are set.
#  Variable               Type      Cached  Description
#  ========               ====      ======  ===========
#  CMAKE_INSTALL_PREFIX   path      yes     Installation prefix
#  CMAKE_MODULE_PATH      path      yes     Path to LOFAR CMake module files
#  CMAKE_<LANG>_COMPILER  filepath  yes     Compiler to use for <LANG>, where
#                                           <LANG> is usually C and C++
#  LOFAR_ROOT             internal  yes     Root directory of LOFAR source tree
#  LOFAR_COMPILER_SUITE   internal  yes     Compiler suite (e.g., gnu),
#                                           derived from CMAKE_BINARY_DIR
#  LOFAR_BUILD_VARIANT    internal  yes     Build variant (e.g., debug), 
#                                           derived from CMAKE_BINARY_DIR
#  LOFAR_LIBDIR           internal  yes     Directory where libraries will be
#                                           installed, lib or lib64.
#
#  for backward compatibility:
#  --------------------------
#  lofar_top_srcdir       internal  yes     Root directory of LOFAR source tree
#  lofar_sharedir         internal  yes     $lofar_top_srcdir/autoconf_share
#  prefix                 internal  yes     Install prefix
#

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

if(NOT DEFINED LOFAR_INIT_INCLUDED)

  set(LOFAR_INIT_INCLUDED TRUE)

  # Bail out if the user is doing an in-source build.
  if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR 
      "You've attempted to do an in-source build. Please remove the cache "
      "file and re-run CMake outside your source tree.")
  endif("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")

  # Bail out if there's a cache file in the source directory.
  if(EXISTS "${CMAKE_SOURCE_DIR}/CMakeCache.txt")
    message(FATAL_ERROR 
      "CMake found a cache file in your source tree. Please remove it "
      "manually and re-run CMake outside your source tree.")
  endif(EXISTS "${CMAKE_SOURCE_DIR}/CMakeCache.txt")

  # Root directory of the LOFAR source code tree
  set(LOFAR_ROOT ${CMAKE_SOURCE_DIR} CACHE INTERNAL "LOFAR root directory")

  # Here's where we keep our own CMake modules.
  set(CMAKE_MODULE_PATH "${LOFAR_ROOT}/CMake" CACHE PATH 
    "LOFAR CMake module path")

  # Get compiler suite and build variant from binary directory name.
  # The directory name should follow the naming convention
  # <compiler>_<variant>, where <compiler> specifies the compiler suite to
  # use, and <variant> specifies the build variant (e.g., debug).
  get_filename_component(_cmpvar ${CMAKE_BINARY_DIR} NAME)

  # Set the default install path prefix.
  set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/installed" CACHE PATH 
    "Install path prefix")

  # Split directory name in compiler suite part and build variant part.
  string(TOUPPER ${_cmpvar} _cmpvar)
  string(REGEX REPLACE "\(.*)_.*" "\\1" _cmp ${_cmpvar})
  string(REGEX REPLACE ".*_\(.*)" "\\1" _var ${_cmpvar})

  # Include the compiler definition file
  include(variants/${_cmp} OPTIONAL RESULT_VARIABLE _result)
  if(_result)
    message(STATUS "Loaded compiler defintion file for ${_cmp}")
  endif(_result)

  # Include host-specific variants file, if present, and global variants file.
  include(LofarVariants)

  # Check if compiler suite is known. Compiler suites should be defined in the
  # variants file.
  list(FIND LOFAR_COMPILER_SUITES ${_cmp} _index)
  if(_index GREATER -1)
    set(LOFAR_COMPILER_SUITE ${_cmp} CACHE INTERNAL 
      "Compiler suite, options are ${LOFAR_COMPILER_SUITES}")
  else(_index GREATER -1)
    message(FATAL_ERROR
      "Compiler suite ${_cmp} is not defined, check your variants file!")
  endif(_index GREATER -1)

  # Get the list of compilers for this compiler suite.
  set(_compilers "${LOFAR_COMPILER_SUITE}_COMPILERS")
  if(NOT DEFINED ${_compilers})
    message(FATAL_ERROR 
      "${_compilers} is not defined. Check your variants file!")
  endif(NOT DEFINED ${_compilers})

  # Check if build variant is known. Build variants should be defined in the
  # variants file.
  list(FIND LOFAR_BUILD_VARIANTS ${_var} _index)
  if(_index GREATER -1)
    set(LOFAR_BUILD_VARIANT ${_var} CACHE INTERNAL
      "Build variant, options are ${LOFAR_BUILD_VARIANTS}")
  else(_index GREATER -1)
    message(FATAL_ERROR
      "Build variant ${_var} is not defined. Check your variants file!")
  endif(_index GREATER -1)

  # Define all the available build types. 
  set(CMAKE_CONFIGURATION_TYPES ${LOFAR_BUILD_VARIANTS} CACHE INTERNAL
    "Specify the available build types" FORCE)

  # Set the correct build type. The build type influences which compiler flags
  # will be supplied by CMake (see below).
  set(CMAKE_BUILD_TYPE "${LOFAR_BUILD_VARIANT}" CACHE STRING 
    "Set the correct build type, options are ${LOFAR_BUILD_VARIANTS}" FORCE)

  # Set the CMAKE_<LANG>_COMPILER and the CMAKE_<LANG>_FLAGS_<BUILD_TYPE>
  # variables. These variables are used by CMake in choosing the
  # appropiate compiler and supplying the correct compiler flags depending on
  # the build variant (e.g. debug or opt). These are all cache variables whose
  # values must be forced to the values specified in our variants file.
  foreach(_cmp ${${_compilers}})
    string(REGEX REPLACE "${LOFAR_COMPILER_SUITE}_" "" _lang ${_cmp})
    message(STATUS "${_lang} compiler: ${${_cmp}}")
    set(CMAKE_${_lang}_COMPILER ${${_cmp}} CACHE FILEPATH 
      "${_lang} compiler." FORCE)
    set(CMAKE_${_lang}_FLAGS ${${_cmp}_FLAGS} CACHE STRING 
      "Flags used by the compiler for all build types." FORCE)
    foreach(_var ${LOFAR_BUILD_VARIANTS})
      set(CMAKE_${_lang}_FLAGS_${_var} ${${_cmp}_FLAGS_${_var}} CACHE STRING
        "Flags used by the compiler for ${_var} builds." FORCE)
    endforeach(_var ${LOFAR_BUILD_VARIANTS})
  endforeach(_cmp ${_compilers})

  # Set the CMAKE_EXE_LINKER_FLAGS, CMAKE_EXE_LINKER_FLAGS_<BUILD_TYPE>,
  # CMAKE_SHARED_LINKER_FLAGS and CMAKE_SHARED_LINKER_FLAGS_<BUILD_TYPE>
  # variables. These variables are used by CMake to supply the correct link
  # flags depending on the build variant (e.g. debug or opt). These are all
  # cache variables whose values must be forced to the values specified in our
  # variants file.
  set(CMAKE_EXE_LINKER_FLAGS ${${LOFAR_COMPILER_SUITE}_EXE_LINKER_FLAGS}
    CACHE STRING "Flags used by the linker for all build types to create executables." FORCE)
  set(CMAKE_SHARED_LINKER_FLAGS ${${LOFAR_COMPILER_SUITE}_SHARED_LINKER_FLAGS}
    CACHE STRING "Flags used by the linker for all build types to create shared libraries." FORCE)
  foreach(_var ${LOFAR_BUILD_VARIANTS})
    set(CMAKE_EXE_LINKER_FLAGS_${_var} 
      ${${LOFAR_COMPILER_SUITE}_EXE_LINKER_FLAGS_${_var}} CACHE STRING
      "Flags used by the linker for ${_var} builds to create executables." FORCE)
    set(CMAKE_SHARED_LINKER_FLAGS_${_var} 
      ${${LOFAR_COMPILER_SUITE}_SHARED_LINKER_FLAGS_${_var}} CACHE STRING
      "Flags used by the linker for ${_var} builds to create shared libraries." FORCE)
  endforeach(_var ${LOFAR_BUILD_VARIANTS})

  # Set compiler definitions (e.g., -D options). There are global options that
  # apply to each build variant, and there are build variant-specific options.
  # Use separate_arguments to convert spaces to semicolons first.
  separate_arguments(${LOFAR_COMPILER_SUITE}_COMPILE_DEFINITIONS)
  separate_arguments(${LOFAR_COMPILER_SUITE}_COMPILE_DEFINITIONS_${LOFAR_BUILD_VARIANT})
  add_definitions(${${LOFAR_COMPILER_SUITE}_COMPILE_DEFINITIONS})
  add_definitions(${${LOFAR_COMPILER_SUITE}_COMPILE_DEFINITIONS_${LOFAR_BUILD_VARIANT}})

  # Determine if libraries have to be put in lib or lib64. On Debian systems
  # they have to be put in lib, on non-Debian 64-bit systems in lib64.
  set(_libdir lib)
  if(NOT EXISTS /etc/debian_version AND EXISTS /usr/lib64)
    set(_libdir lib64)
  endif(NOT EXISTS /etc/debian_version AND EXISTS /usr/lib64)
  set(LOFAR_LIBDIR ${_libdir} CACHE INTERNAL
    "Directory where libraries will be installed")

  # Create a directory structure in the build directory similar to that in the
  # install directory. Binaries should be put in bin or sbin, libraries in lib
  # or lib64 (dependent on architecture), configuration files in etc or share,
  # and run-time files in var. The include directory will contain symbolic links
  # to all (sub)projects of the current build. This is needed, because we use
  # #include's that all contain the names of the different subprojects
  # (e.g. Common, Blob).
  foreach(_dir bin etc include ${LOFAR_LIBDIR} sbin share var)
    execute_process(COMMAND ${CMAKE_COMMAND} -E 
      make_directory ${CMAKE_BINARY_DIR}/${_dir})
  endforeach(_dir bin sbin include ${LOFAR_LIBDIR})
  

  ## --------------------------------------------------------------------------
  ## Several "Auto-tools variables" needed for backward compatibility
  ## --------------------------------------------------------------------------
  set(lofar_top_srcdir "${LOFAR_ROOT}" CACHE INTERNAL "lofar_top_srcdir")
  set(lofar_sharedir "${lofar_top_srcdir}/autoconf_share" CACHE INTERNAL "lofar_sharedir")
  set(prefix "${CMAKE_INSTALL_PREFIX}" CACHE INTERNAL "prefix")

endif(NOT DEFINED LOFAR_INIT_INCLUDED)
