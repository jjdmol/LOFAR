# - Handle CMake options and set associated variables.

#  Copyright (C) 2008-2010
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

if(NOT DEFINED LOFAR_OPTIONS_INCLUDED)

  set(LOFAR_OPTIONS_INCLUDED TRUE)

  ## --------------------------------------------------------------------------
  ## Include wrapper macro for find_package()
  ## --------------------------------------------------------------------------
  include(LofarFindPackage)

  ## --------------------------------------------------------------------------
  ## Handle contradicting options
  ## --------------------------------------------------------------------------
  if(BUILD_STATIC_EXECUTABLES AND BUILD_SHARED_LIBS)
    message(FATAL_ERROR 
      "Cannot create static executables, when creating shared libraries. "
      "Please check your variants file!")
  endif(BUILD_STATIC_EXECUTABLES AND BUILD_SHARED_LIBS)

  if(USE_LOG4CXX AND USE_LOG4CPLUS)
    message(FATAL_ERROR 
      "You cannot use more than one logger implementation. "
      "Please check your variants file!")
  endif(USE_LOG4CXX AND USE_LOG4CPLUS)

  if(USE_OPENMP AND NOT USE_THREADS)
    message(FATAL_ERROR
      "Using OpenMP implies using threads. "
      "Please check your variants file!")
  endif(USE_OPENMP AND NOT USE_THREADS)

  ## --------------------------------------------------------------------------
  ## Handle each option
  ## --------------------------------------------------------------------------
  if(BUILD_STATIC_EXECUTABLES)
    set(CMAKE_EXE_LINKER_FLAGS -static)
    set(CMAKE_FIND_LIBRARY_SUFFIXES .a)
    set(CMAKE_EXE_LINK_DYNAMIC_C_FLAGS)       # remove -Wl,-Bdynamic
    set(CMAKE_EXE_LINK_DYNAMIC_CXX_FLAGS)
    set(CMAKE_SHARED_LIBRARY_C_FLAGS)         # remove -fPIC
    set(CMAKE_SHARED_LIBRARY_CXX_FLAGS)
    set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS)    # remove -rdynamic
    set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS)
    # Maybe this works as well, haven't tried yet.
    # set_property(GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS FALSE)
    # See http://www.mail-archive.com/cmake@cmake.org/msg21473.html
  else(BUILD_STATIC_EXECUTABLES)
    # Set RPATH to use for installed targets; append linker search path
    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${LOFAR_LIBDIR}")
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
  endif(BUILD_STATIC_EXECUTABLES)
  
  if(USE_BACKTRACE)
    lofar_find_package(Backtrace REQUIRED)
  else(USE_BACKTRACE)
    set(HAVE_BACKTRACE OFF)
  endif(USE_BACKTRACE)

  if(USE_LOG4CPLUS)
    lofar_find_package(Log4Cplus REQUIRED)
  else(USE_LOG4CPLUS)
    set(HAVE_LOG4CPLUS OFF)
  endif(USE_LOG4CPLUS)

  if(USE_LOG4CXX)
    lofar_find_package(Log4Cxx REQUIRED)
  else(USE_LOG4CXX)
    set(HAVE_LOG4CXX OFF)
  endif(USE_LOG4CXX)

  if(USE_MPI)
    lofar_find_package(MPI REQUIRED)
  else(USE_MPI)
    set(HAVE_MPI OFF)
  endif(USE_MPI)
  
  if(USE_OPENMP)
    lofar_find_package(OpenMP REQUIRED)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
  else(USE_OPENMP)
    set(HAVE_OPENMP OFF)
  endif(USE_OPENMP)

  if(USE_SHMEM)
    set(HAVE_SHMEM 1)
  endif(USE_SHMEM)

  if(NOT USE_SOCKETS)
    add_definitions(-DUSE_NOSOCKETS)
  endif(NOT USE_SOCKETS)

  if(USE_THREADS)
    set(_errmsg "FIXME: Don't know how to enable thread support for ")
    lofar_find_package(Pthreads REQUIRED)
    if(CMAKE_C_COMPILER_ID MATCHES "GNU" OR 
       CMAKE_C_COMPILER_ID MATCHES "Clang")
      set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")
    else()
      message(FATAL_ERROR "${_errmsg} ${CMAKE_C_COMPILER}")
    endif()
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR 
       CMAKE_CXX_COMPILER_ID MATCHES "Clang")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
    else()
      message(FATAL_ERROR "${_errmsg} ${CMAKE_CXX_COMPILER}")
    endif()
    add_definitions(-DUSE_THREADS)
  endif(USE_THREADS)

endif(NOT DEFINED LOFAR_OPTIONS_INCLUDED)
