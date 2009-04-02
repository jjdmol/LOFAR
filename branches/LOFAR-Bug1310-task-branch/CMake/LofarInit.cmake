#  $Id$
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


#  Initialize common LOFAR CMake variables.
#
#  The following variables and properties are set.
#
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
#
#  for backward compatibility:
#  --------------------------
#  lofar_top_srcdir       internal  yes     Root directory of LOFAR source tree
#  lofar_sharedir         internal  yes     $lofar_top_srcdir/autoconf_share
#  prefix                 internal  yes     Install prefix
#
#
#  Property                         Scope   Description
#  ========                         =====   ===========
#  FIND_LIBRARY_USE_LIB64_PATHS     global  Set to true if either ENABLE_LIB64
#                                           is set or directory /lib64 exists,
#                                           otherwise set to false.

if(NOT DEFINED LOFAR_INIT_INCLUDED)

  message(STATUS "**** ENTER: LofarInit.cmake ****")

  set(LOFAR_INIT_INCLUDED TRUE)

  # Root directory of the LOFAR source code tree
  string(REGEX REPLACE 
    "^(.*/LOFAR)/.*$" "\\1" LOFAR_ROOT ${CMAKE_SOURCE_DIR})
  set(LOFAR_ROOT ${LOFAR_ROOT} CACHE INTERNAL "LOFAR root directory")

  # Here's where we keep our own CMake modules.
  set(CMAKE_MODULE_PATH "${LOFAR_ROOT}/CMake" CACHE PATH "LOFAR CMake module path")

  # Include host-specific variants file, if present, and global variants file.
  include(LofarVariants)

  # Get compiler suite and build variant from binary directory name.
  # The directory name should follow the naming convention
  # <compiler>_<variant>, where <compiler> specifies the compiler suite to
  # use, and <variant> specifies the build variant (e.g., debug).
  get_filename_component(cmpvar ${CMAKE_BINARY_DIR} NAME)

  # Set the default install path prefix.
  set(CMAKE_INSTALL_PREFIX "${LOFAR_ROOT}/installed/${cmpvar}" CACHE PATH "Instal path prefix")

  # Split directory name in compiler suite part and build variant part.
  string(TOUPPER ${cmpvar} cmpvar)
  string(REGEX REPLACE "\(.*)_.*" "\\1" cmp ${cmpvar})
  string(REGEX REPLACE ".*_\(.*)" "\\1" var ${cmpvar})

  # Check if compiler suite is known. Compiler suites should be defined in the
  # variants file.
  if(LOFAR_COMPILER_SUITES MATCHES ${cmp})
    set(LOFAR_COMPILER_SUITE ${cmp} CACHE INTERNAL 
      "Compiler suite (e.g., gnu)")
  else(LOFAR_COMPILER_SUITES MATCHES ${cmp})
    message(FATAL_ERROR
      "Compiler suite ${cmp} is not defined. Check your variants file!")
  endif(LOFAR_COMPILER_SUITES MATCHES ${cmp})
  
  # Get the list of compilers for this compiler suite.
  set(compilers "${LOFAR_COMPILER_SUITE}_COMPILERS")
  if(NOT DEFINED ${compilers})
    message(FATAL_ERROR 
      "${compilers} is not defined. Check your variants file!")
  endif(NOT DEFINED ${compilers})

  # Check if build variant is known. Build variants should be defined in the
  # variants file.
  message(STATUS "LOFAR_BUILD_VARIANTS = ${LOFAR_BUILD_VARIANTS}")
  if(LOFAR_BUILD_VARIANTS MATCHES ${var})
    set(LOFAR_BUILD_VARIANT ${var} CACHE INTERNAL
      "Build variant, options are ${LOFAR_BUILD_VARIANTS}")
  else(${LOFAR_BUILD_VARIANTS} MATCHES ${var})
    message(FATAL_ERROR
      "Build variant ${var} is not defined. Check your variants file!")
  endif(LOFAR_BUILD_VARIANTS MATCHES ${var})

  # Define all the available build types. 
  set(CMAKE_CONFIGURATION_TYPES ${LOFAR_BUILD_VARIANTS})

  # Set the correct build type. The build type influences which compiler flags
  # will be supplied by CMake (see below).
  set(CMAKE_BUILD_TYPE "${LOFAR_BUILD_VARIANT}" CACHE STRING 
    "Set the correct build type, options are ${LOFAR_BUILD_VARIANTS}" FORCE)

  # Set the CMAKE_<LANG>_COMPILER and the CMAKE_<LANG>_FLAGS_<BUILD_TYPE>
  # variables. These variables are used by CMake in choosing the
  # appropiate compiler and supplying the correct compiler flags depending on
  # the build variant (e.g. debug or opt). These are all cache variables whose
  # values must be forced to the values specified in our variants file.
  foreach(cmp ${${compilers}})
    string(REGEX REPLACE "${LOFAR_COMPILER_SUITE}_" "" lang ${cmp})
    message(STATUS "${lang} compiler: ${${cmp}}")
    set(CMAKE_${lang}_COMPILER ${${cmp}} CACHE FILEPATH 
      "${lang} compiler." FORCE)
    set(CMAKE_${lang}_FLAGS ${${cmp}_FLAGS} CACHE STRING 
      "Flags used by the compiler during all build types." FORCE)
    foreach(var ${LOFAR_BUILD_VARIANTS})
      set(CMAKE_${lang}_FLAGS_${var} ${${cmp}_FLAGS_${var}} CACHE STRING
        "Flags used by the compiler during ${var} builds." FORCE)
    endforeach(var ${LOFAR_BUILD_VARIANTS})
  endforeach(cmp ${compilers})

  # Set compiler definitions (e.g., -D options). There are global options that
  # apply to each build variant, and there are build variant-specific options.
  add_definitions(${${LOFAR_COMPILER_SUITE}_COMPILE_DEFINITIONS})
  foreach(var ${LOFAR_BUILD_VARIANTS})
    add_definitions(${${LOFAR_COMPILER_SUITE}_COMPILE_DEFINITIONS_${var}})
  endforeach(var ${LOFAR_BUILD_VARIANTS})

  # Determine if lib64 has to be used. This is somewhat tricky for the default
  # case, because: Debian puts 64-bit libs in /lib and 32-bit libs in /lib32,
  # but SuSE and RedHat put 64-bit libs in /lib64 and 32-bit libs in /lib.
  # We cannot use `uname -s`, since all distros return "Linux", hence we must
  # test for the presence of these lib directories instead. To further
  # complicate matters, Debian distros may create a symlink /lib64; therefore
  # we should make sure that /lib64 is a "real" directory, not a symlink.
  set(LOFAR_LIBDIR lib)
  if(DEFINED ENABLE_LIB64)
    if(ENABLE_LIB64)
      set(LOFAR_LIBDIR lib64)
    else(ENABLE_LIB64)
      execute_process(COMMAND test -d /lib32 -a ! -L /lib32 RESULT_VARIABLE result)
      if(NOT result)
        set(LOFAR_LIBDIR lib32)
      endif(NOT result)
    endif(ENABLE_LIB64)
  else(DEFINED ENABLE_LIB64)
    execute_process(COMMAND test -d /lib64 -a ! -L /lib64 RESULT_VARIABLE result)
    if(NOT result)
      set(LOFAR_LIBDIR lib64)
    endif(NOT result)
  endif(DEFINED ENABLE_LIB64)

  if(LOFAR_LIBDIR STREQUAL lib64)
    set_property(GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS TRUE)
  else(LOFAR_LIBDIR STREQUAL lib64)
    set_property(GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS FALSE)
  endif(LOFAR_LIBDIR STREQUAL lib64)

  # Create include directory that will hold symbolic links to all
  # (sub)projects of the current build. This is needed, because we use
  # #include's that all contain the names of the different subprojects
  # (e.g. Common, Blob).
  if(NOT EXISTS ${CMAKE_BINARY_DIR}/include)
    execute_process(COMMAND ${CMAKE_COMMAND} -E 
      make_directory ${CMAKE_BINARY_DIR}/include)
  endif(NOT EXISTS ${CMAKE_BINARY_DIR}/include)

  # Add a custom target, so that we can do a 'make check', like we did with
  # the GNU Autotools. 
  # Tests should be added with:
  #   - add_executable(<testprog> EXCLUDE_FROM_ALL ...) 
  #   - add_dependencies(check <testprog>)
  add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND})

  ## --------------------------------------------------------------------------
  ## Several "Auto-tools variables" needed for backward compatibility
  ## --------------------------------------------------------------------------
  set(lofar_top_srcdir "${LOFAR_ROOT}" CACHE INTERNAL "lofar_top_srcdir")
  set(lofar_sharedir "${lofar_top_srcdir}/autoconf_share" CACHE INTERNAL "lofar_sharedir")
  set(prefix "${CMAKE_INSTALL_PREFIX}" CACHE INTERNAL "prefix")

  message(STATUS "**** LEAVE: LofarInit.cmake ****")

endif(NOT DEFINED LOFAR_INIT_INCLUDED)
