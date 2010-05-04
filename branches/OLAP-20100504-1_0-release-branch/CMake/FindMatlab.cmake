# - Find the Matlab compiler, include directory and run-time library
# This macro tries to find MCC - the Matlab to C/C++ Compiler. Furthermore,
# it tries to determine the path to the C/C++ include files and the Matlab
# C/C++ Runtime Library.
#
# The following variables are set by this module:
#  MATLAB_COMPILER            path to the Matlab to C/C++ Compiler
#  MATLAB_INCLUDE_DIR         directory containing the file mclmcrrt.h
#  MATLAB_MWMCLMCRRT_LIBRARY  path to the Matlab C/C++ Runtime Library
#                             libmwmclmcrrt.so.
# The user can specify MATLAB_ROOT_DIR as a hint to the find_XXX() calls,
# ensuring that MATLAB_ROOT_DIR is searched before any system directories.

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


if(NOT MATLAB_FOUND)

  # Find MCC - the Matlab to C/C++ Compiler
  find_program(MATLAB_COMPILER mcc
    HINTS ${MATLAB_ROOT_DIR}/bin
    DOC "Matlab to C/C++ Compiler")

  # Find the directory containing the header file mclmcrrt.h
  find_path(MATLAB_INCLUDE_DIR
    mclmcrrt.h
    HINTS ${MATLAB_ROOT_DIR}/extern/include
    DOC "Matlab C/C++ include directory")

  # Find libmwmclmcrrt.so - the Matlab C/C++ Runtime Library
  find_library(MATLAB_MWMCLMCRRT_LIBRARY
    mwmclmcrrt
    HINTS ${MATLAB_ROOT_DIR}/bin/glnx86
    DOC "Matlab C/C++ Runtime Library")

  # Handle the QUIETLY and REQUIRED arguments and set MATLAB_FOUND to TRUE if
  # all listed variables are TRUE
  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(Matlab DEFAULT_MSG 
    MATLAB_COMPILER MATLAB_INCLUDE_DIR MATLAB_MWMCLMCRRT_LIBRARY)
  mark_as_advanced(
    MATLAB_COMPILER MATLAB_INCLUDE_DIR MATLAB_MWMCLMCRRT_LIBRARY)

  if(MATLAB_FOUND)
    set(MATLAB_LIBRARIES ${MATLAB_MWMCLMCRRT_LIBRARY})
    set(MATLAB_INCLUDE_DIRS ${MATLAB_INCLUDE_DIR})
  endif(MATLAB_FOUND)

endif(NOT MATLAB_FOUND)
