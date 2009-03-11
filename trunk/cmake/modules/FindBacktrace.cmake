# +-----------------------------------------------------------------------------+
# | $Id:: IO.h 393 2007-06-13 10:49:08Z gels                                  $ |
# +-----------------------------------------------------------------------------+
# |   Copyright (C) 2007                                                        |
# |   Martin Gels (gels@astron.nl)                                           |
# |                                                                             |
# |   This program is free software; you can redistribute it and/or modify      |
# |   it under the terms of the GNU General Public License as published by      |
# |   the Free Software Foundation; either version 2 of the License, or         |
# |   (at your option) any later version.                                       |
# |                                                                             |
# |   This program is distributed in the hope that it will be useful,           |
# |   but WITHOUT ANY WARRANTY; without even the implied warranty of            |
# |   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             |
# |   GNU General Public License for more details.                              |
# |                                                                             |
# |   You should have received a copy of the GNU General Public License         |
# |   along with this program; if not, write to the                             |
# |   Free Software Foundation, Inc.,                                           |
# |   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.                 |
# +-----------------------------------------------------------------------------+

# Check for the presence of BACKTRACE (BACKTRACE).
#
# The following variables are set when BACKTRACE is found:
#  Backtrace_FOUND      = Set to true, if all components of BACKTRACE have been found.
#  BACKTRACE_INCLUDES   = Include path for the header files of BACKTRACE
#  BACKTRACE_LIBRARIES  = Link these to use BACKTRACE
#  BACKTRACE_LFGLAS     = Linker flags (optional)
# The following variables provide more detais information on the components:
#  HAVE_DEMANGLE_H
#  HAVE_EXECINFO_H
#  HAVE_BFD_H

## -----------------------------------------------------------------------------
## Search locations & macros

include (CMakeSettings)
include (CheckFunctionExists)

## -----------------------------------------------------------------------------
## Check for the header files

set (BACKTRACE_INCLUDES "")
set (BACKTRACE_LIBRARIES "")

find_path (HAVE_DEMANGLE_H demangle.h
  PATHS ${include_locations}
  NO_DEFAULT_PATH
  )

find_path (HAVE_EXECINFO_H execinfo.h
  PATHS ${include_locations}
  NO_DEFAULT_PATH
  )

find_path (HAVE_BFD_H bfd.h
  PATHS ${include_locations}
  NO_DEFAULT_PATH
  )

## logic for combination of header files and libraries

if (HAVE_DEMANGLE_H)
  list (APPEND BACKTRACE_INCLUDES ${HAVE_DEMANGLE_H})
endif (HAVE_DEMANGLE_H)

if (HAVE_EXECINFO_H)
  ## append location of header file
  list (APPEND BACKTRACE_INCLUDES ${HAVE_EXECINFO_H})

  ## search for related libraries
  find_library (HAVE_LIBIBERTY iberty
    PATHS ${lib_locations}
    NO_DEFAULT_PATH
  )
  if (HAVE_LIBIBERTY)
    list (APPEND CMAKE_REQUIRED_LIBRARIES ${HAVE_LIBIBERTY})
  endif (HAVE_LIBIBERTY)
  
  SET (Bactrace_LIB_PREFIX "lib")
  find_library (HAVE_LIBZ ${Bactrace_LIB_PREFIX}z.a
    PATHS ${lib_locations}
    NO_DEFAULT_PATH
  )
  if (HAVE_LIBZ)
    list (APPEND CMAKE_REQUIRED_LIBRARIES ${HAVE_LIBZ})
  endif (HAVE_LIBZ)

  find_library (HAVE_LIBBFD bfd
    PATHS ${lib_locations}
    NO_DEFAULT_PATH
  )
  if (HAVE_LIBBFD)
    list (APPEND CMAKE_REQUIRED_LIBRARIES ${HAVE_LIBBFD})
  endif (HAVE_LIBBFD)
   
  if (HAVE_BFD_H)
    ## append location of header file
    list (APPEND BACKTRACE_INCLUDES ${HAVE_BFD_H})
    ## check if functions exist in library
    if (HAVE_LIBBFD)
      list (APPEND BACKTRACE_LIBRARIES ${HAVE_LIBBFD})
      check_function_exists (bfd_init HAVE_BFD_INIT)
      if (NOT HAVE_BFD_INIT)
	message (STATUS "Warning: bfd_init not found, please install the GNU binutils!")
      else(NOT HAVE_BFD_INIT)
        add_definitions (-DHAVE_BFD)
      endif(NOT HAVE_BFD_INIT)
      
      check_function_exists (cplus_demangle HAVE_CPLUS_DEMANGLE)
      if (HAVE_CPLUS_DEMANGLE)
	add_definitions (-DHAVE_CPLUS_DEMANGLE)
      else (HAVE_CPLUS_DEMANGLE)
	message (STATUS "Warning: cplus_demangle not found, please install the GNU binutils!")
      endif(HAVE_CPLUS_DEMANGLE)
    endif (HAVE_LIBBFD)
  endif (HAVE_BFD_H)

  ## check if functions exist in header
  check_function_exists (backtrace HAVE_BACKTRACEFUNC)
  if (NOT HAVE_BACKTRACEFUNC)
    message (STATUS "Warning: backtrace not found in glibc!") 
  endif (NOT HAVE_BACKTRACEFUNC)
  
  ## check if functions exist in library
  if (HAVE_LIBIBERTY)
    list (APPEND BACKTRACE_LIBRARIES ${HAVE_LIBIBERTY})
    check_function_exists (xexit HAVE_XEXIT)
  endif (HAVE_LIBIBERTY)
  if (HAVE_LIBZ)
    list (APPEND BACKTRACE_LIBRARIES ${HAVE_LIBZ})
    check_function_exists (inflate HAVE_INFLATE)
  endif (HAVE_LIBZ)
else (HAVE_EXECINFO_H)
  message (SEND_ERROR "execinfo.h not found, please install glibc-devel!")
endif (HAVE_EXECINFO_H)

## -----------------------------------------------------------------------------
## Actions taken when all components have been found

if (BACKTRACE_INCLUDES AND BACKTRACE_LIBRARIES)
  set (Backtrace_FOUND TRUE)
else (BACKTRACE_INCLUDES AND BACKTRACE_LIBRARIES)
  set (Backtrace_FOUND FALSE)
  if (NOT BACKTRACE_FIND_QUIETLY)
    if (NOT BACKTRACE_INCLUDES)
      message (STATUS "Unable to find BACKTRACE header files!")
    endif (NOT BACKTRACE_INCLUDES)
    if (NOT BACKTRACE_LIBRARIES)
      message (STATUS "Unable to find BACKTRACE library files!")
    endif (NOT BACKTRACE_LIBRARIES)
  endif (NOT BACKTRACE_FIND_QUIETLY)
endif (BACKTRACE_INCLUDES AND BACKTRACE_LIBRARIES)

if (Backtrace_FOUND)
  if (NOT BACKTRACE_FIND_QUIETLY)
    message (STATUS "Found components for BACKTRACE")
    message (STATUS "BACKTRACE_INCLUDES  = ${BACKTRACE_INCLUDES}")
    message (STATUS "BACKTRACE_LIBRARIES = ${BACKTRACE_LIBRARIES}")
  endif (NOT BACKTRACE_FIND_QUIETLY)
else (Backtrace_FOUND)
  if (BACKTRACE_FIND_REQUIRED)
    message (FATAL_ERROR "Could not find BACKTRACE!")
  endif (BACKTRACE_FIND_REQUIRED)
endif (Backtrace_FOUND)

## -----------------------------------------------------------------------------
## Mark advanced variables

mark_as_advanced (
  BACKTRACE_INCLUDES
  BACKTRACE_LIBRARIES
  )
