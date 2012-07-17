# - A tiny wrapper around the FindBLAS.cmake macro that comes with CMake.
# Its purpose is to wrap the enable_langauage(Fortran) command. If BLAS is
# required, then the Fortran compiler must also be available. Otherwise, it is
# not an error if a Fortran compiler is missing.

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

# Enable the Fortran compiler, if that has not been done yet.
get_property(_enabled_languages GLOBAL PROPERTY ENABLED_LANGUAGES)
if(NOT _enabled_languages MATCHES Fortran)
  # Work-around for CMake issue #9220
  if(CMAKE_Fortran_COMPILER MATCHES "^$")
    set(CMAKE_Fortran_COMPILER CMAKE_Fortran_COMPILER-NOTFOUND)
  endif(CMAKE_Fortran_COMPILER MATCHES "^$")
  if(BLAS_FIND_REQUIRED)
    enable_language(Fortran)
  else(BLAS_FIND_REQUIRED)
    enable_language(Fortran OPTIONAL)
  endif(BLAS_FIND_REQUIRED)
endif(NOT _enabled_languages MATCHES Fortran)

# If we have a working Fortran compiler, call the "real" FindBLAS module;
# otherwise display a diagnostic message.
if(CMAKE_Fortran_COMPILER_WORKS)
  include(${CMAKE_ROOT}/Modules/FindBLAS.cmake)
else(CMAKE_Fortran_COMPILER_WORKS)
  if(BLAS_FIND_REQUIRED)
    message(SEND_ERROR "FindBLAS requires a working Fortran compiler!")
  else(BLAS_FIND_REQUIRED)
    message(STATUS "FindBLAS requires a working Fortran compiler!")
  endif(BLAS_FIND_REQUIRED)
endif(CMAKE_Fortran_COMPILER_WORKS)
