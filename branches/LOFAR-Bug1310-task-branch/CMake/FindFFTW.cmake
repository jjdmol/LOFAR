# $Id$
#
# Copyright (C) 2008-2009
# ASTRON (Netherlands Foundation for Research in Astronomy)
# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

# Try to find FFTW.
#
# This will define:
#  
#  FFTW_FOUND        - FFTW is present for the requested component.
#  FFTW_COMPONENT    - FFTW component; only set when component was found.
#                      (cached)
#  FFTW_INCLUDE_DIR  - the FFTW include directory for the requested component 
#                      (cached)
#  FFTW_INCLUDE_DIRS - the FFTW include directories
#                      (identical to FFTW_INCLUDE_DIR)
#  FFTW_LIBRARY      - the FFTW library for the requested component 
#                      (cached)
#  FFTW_LIBRARIES    - the FFTW libraries
#                      (identical to FFTW_LIBRARY)

set(_usage_msg 
  "Usage: find_package(FFTW [COMPONENTS [real] [double|single] [mpi|threads]")

# -----------------------------------------------------------------------------
# Get the optional `type' component: [real]. 
# The output will be stored in the variable _fftw_type.
# -----------------------------------------------------------------------------
macro(get_fftw_type)
  set(_fftw_type)
  list(FIND FFTW_FIND_COMPONENTS real _idx)
  if(_idx GREATER -1)
    set(_fftw_type real)
  endif(_idx GREATER -1)
endmacro(get_fftw_type)

# -----------------------------------------------------------------------------
# Get the optional component ${_kind}. Sensible values for ${_kind} are (at
# the moment) precision, or parallelization.
# The variable _options will contain the list of valid components for
# ${_kind}; the first element of _options is used as default value, in case no
# matching component could be found.
# The output will be stored in the variable _fftw_${_kind}.
#
# It is an error if, between multiple calls of FindFFTW(), the currently
# specified component value is different from the cached one.
#
# Usage get_fftw_component(<kind> <default-option> [option] ...)
# -----------------------------------------------------------------------------
macro(get_fftw_component _kind)
  set(_options ${ARGN})
  if(_options)
    list(GET _options 0 _default_option)
  endif(_options)
  set(_fftw_${_kind})
  foreach(_opt ${_options})
    list(FIND FFTW_FIND_COMPONENTS ${_opt} _idx)
    if(_idx GREATER -1)
      if(NOT _fftw_${_kind})
        set(_fftw_${_kind} ${_opt})
      else(NOT _fftw_${_kind})
        message(FATAL_ERROR
          "FindFFTW: more than one `${_kind}' component was specified.\n"
          "${_usage_msg}")
      endif(NOT _fftw_${_kind})
    endif(_idx GREATER -1)
  endforeach(_opt ${_options})
  if(NOT _fftw_${_kind})
    set(_fftw_${_kind} "${_default_option}")
  endif(NOT _fftw_${_kind})
  string(TOUPPER "FFTW_${_kind}" _cached_option)
  if(DEFINED ${_cached_option})
    if(NOT "${_fftw_${_kind}}" STREQUAL "${${_cached_option}}")
      message(FATAL_ERROR
        "FindFFTW: previous call used ${_kind} `${${_cached_option}}', "
        "which is different from `${_fftw_${_kind}}'. This is not supported!")
    endif(NOT "${_fftw_${_kind}}" STREQUAL "${${_cached_option}}")
  endif(DEFINED ${_cached_option})
endmacro(get_fftw_component _kind)

# Get FFTW type (optional: [real])
get_fftw_type()
# Get FFTW precision (optional: [double|single])
get_fftw_component(precision double single)
# Get FFTW parallelization (optional: [off|mpi|threads])
get_fftw_component(parallelization off mpi threads)

# This is the default: complex transforms
set(_libraries fftw)
set(_headerfile fftw.h)

# The real transforms also require rfftw.h and -lrfftw; i.e. prefixed with 'r'.
if(_fftw_type)
  set(_headerfile rfftw.h)
  set(_libraries rfftw ${_libraries})
endif(_fftw_type)

# Parallelization using mpi or threads also requires fftw_mpi.h and lfftw_mpi
# or fftw_threads.h and -lfftw_threads.
if(_fftw_parallelization)
  set(_headerfile fftw_${_fftw_parallelization}.h)
  set(_libraries fftw_${_fftw_parallelization} ${_libraries})
  # And again, if using real transforms, also add the real transform headers
  # and libraries.
  if(_fftw_type)
    set(_headerfile rfftw_${_fftw_parallelization}.h)
    set(_libraries rfftw_${_fftw_parallelization} ${_libraries})
  endif(_fftw_type)
endif(_fftw_parallelization)

# Some distributions choose to prefix the header files and libraries with
# the precision: (s)ingle or (d)ouble.
string(SUBSTRING ${_fftw_precision} 0 1 _prec)

# Search for either prefixed or normal header file.
find_path(FFTW_INCLUDE_DIR NAMES ${_prec}${_headerfile} ${_headerfile})

# Search for all required libraries.
set(FFTW_LIBRARIES)
foreach(_lib ${_libraries})
  string(TOUPPER ${_lib} _LIB)
  message(STATUS "Looking for ${_lib}")
  find_library(${_LIB}_LIBRARY NAMES ${_prec}${_lib}S ${_lib})
  if(${_LIB}_LIBRARY)
    message(STATUS "Looking for ${_lib} - found")
  else(${_LIB}_LIBRARY)
    message(STATUS "Looking for ${_lib} - NOT found")
  endif(${_LIB}_LIBRARY)
  list(APPEND FFTW_LIBRARIES ${${_LIB}_LIBRARY})
endforeach(_lib ${_libraries})

foreach(lib ${FFTW_LIBRARIES})
  if(lib)
    message(STATUS "${lib} is TRUE")
  else(lib)
    message(STATUS "${lib} is FALSE")
  endif(lib)
endforeach(lib ${FFTW_LIBRARIES})

message(STATUS "[FindFFTW] FFTW_INCLUDE_DIR = ${FFTW_INCLUDE_DIR}")
message(STATUS "[FindFFTW] FFTW_LIBRARIES = ${FFTW_LIBRARIES}")

# Handle the QUIETLY and REQUIRED arguments and set FFTW_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FFTW DEFAULT_MSG 
  FFTW_LIBRARIES FFTW_INCLUDE_DIR)

message(STATUS "FFTW_FOUND = ${FFTW_FOUND}")

set(FFTW_PRECISION ${_fftw_precision} CACHE INTERNAL "FFTW precision")
set(FFTW_PARALLELIZATION ${_fftw_parallelization} CACHE INTERNAL "FFTW parallelization")

