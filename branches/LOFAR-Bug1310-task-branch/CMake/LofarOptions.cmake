#  LofarOptions.cmake: Parse CMake options and set associated variables
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
#
#  $Id$

include(LofarFindPackage)

if(NOT DEFINED LOFAR_OPTIONS_INCLUDED)

  message(STATUS "**** ENTER: LofarOptions.cmake ****")

  set(LOFAR_OPTIONS_INCLUDED TRUE)

  if(0)
  ## --------------------------------------------------------------------------
  ## For each USE_<XXX> option that is set, try to find package <XXX>. It is
  ## considered a fatal error, if the package cannot be found.
  ## --------------------------------------------------------------------------
  foreach(opt ${LOFAR_OPTIONS})
    if(NOT DEFINED ${opt})
      message(FATAL_ERROR 
        "Option '${opt}' is a valid LOFAR option, but has not been defined. "
        "Please check your variants file"!)
    endif(NOT DEFINED ${opt})
    message(STATUS "${opt} is ${${opt}}")
    if(${opt})
      string(REGEX REPLACE "^USE_" "" pkg ${opt})
      if(NOT pkg STREQUAL opt)
        message(STATUS "  pkg = ${pkg}")
        lofar_find_package(${pkg} ON)
      endif(NOT pkg STREQUAL opt)
    endif(${opt})
  endforeach(opt ${LOFAR_OPTIONS})
  endif(0)

  if(0)
  foreach(opt ${LOFAR_OPTIONS})
    message(STATUS "opt = ${opt}")
    if(opt MATCHES "^BUILD_SHARED_LIBS$")
      message(STATUS "BUILD_SHARED_LIBS")
    elseif(opt MATCHES "^BUILD_TESTING$")
      message(STATUS "BUILD_TESTING")
    elseif(opt MATCHES "^USE_AIPSPP$")
      message(STATUS "USE_AIPSPP")
    elseif(opt MATCHES "^USE_BACKTRACE$")
      message(STATUS "USE_BACKTRACE")
    elseif(opt MATCHES "^USE_LOG4CPLUS$")
      message(STATUS "USE_LOG4CPLUS")
    elseif(opt MATCHES "^USE_LOG4CXX$")
      message(STATUS "USE_LOG4CXX")
    elseif(opt MATCHES "^USE_PYTHON$")
      message(STATUS "USE_PYTHON")
    elseif(opt MATCHES "^USE_SSE$")
      message(STATUS "USE_SSE")
    elseif(opt MATCHES "^USE_SHMEM$")
      message(STATUS "USE_SHMEM")
    elseif(opt MATCHES "^USE_SOCKETS$")
      message(STATUS "USE_SOCKETS")
    elseif(opt MATCHES "^USE_THREADS$")
      message(STATUS "USE_THREADS")
    elseif(opt MATCHES "^USE_ZOID$")
      message(STATUS "USE_ZOID")
    else()
      message(FATAL_ERROR "Unhandled LOFAR option `${opt}' in LofarOptions.cmake")
    endif()
  endforeach(opt ${LOFAR_OPTIONS})
  endif(0)

## ----------------------------------------------------------------------------

  if(USE_AIPSPP)
    lofar_find_package(CasaCore)
  endif(USE_AIPSPP)
  
  if(USE_BACKTRACE)
    lofar_find_package(Backtrace)
  endif(USE_BACKTRACE)

  if(USE_LOG4CXX AND USE_LOG4CPLUS)
    message(FATAL_ERROR "You cannot use more than one logger implementation. "
      "Please check your variants file!")
  endif(USE_LOG4CXX AND USE_LOG4CPLUS)

  if(USE_LOG4CXX)
    lofar_find_package(Log4Cxx)
  endif(USE_LOG4CXX)

  if(USE_LOG4CPLUS)
    lofar_find_package(Log4Cplus)
  endif(USE_LOG4CPLUS)

  if(USE_PYTHON)
    lofar_find_package(Python)
  endif(USE_PYTHON)

  if(USE_SSE)
    set(GNU_SSE_FLAGS "-msse2")
    set(ICC_SSE_FLAGS "-xW")
  endif(USE_SSE)

  if(USE_SHMEM)
    set(HAVE_SHMEM 1)
  endif(USE_SHMEM)

  if(USE_SOCKETS)
    #
  endif(USE_SOCKETS)

  if(USE_SOCKETS)
    #
  endif(USE_SOCKETS)

  if(USE_THREADS)
    #
  endif(USE_THREADS)

  if(USE_ZOID)
    #
  endif(USE_ZOID)


  message(STATUS "**** LEAVE: LofarOptions.cmake ****")

endif(NOT DEFINED LOFAR_OPTIONS_INCLUDED)
