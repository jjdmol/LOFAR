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

# Check for the presence of SHMEM (SHMEM).
#
# The following variables are set when SHMEM is found:
#  SHMEM_FOUND      = Set to true, if all components of SHMEM have been found.
#  SHMEM_INCLUDES   = Include path for the header files of SHMEM
#  SHMEM_LIBRARIES  = Link these to use SHMEM
#  SHMEM_LFGLAS     = Linker flags (optional)

## -----------------------------------------------------------------------------
## Search locations

include (CMakeSettings)

## -----------------------------------------------------------------------------
## Check for the header files

find_path (SHMEM_INCLUDES LCS/Common/include/Common/shmem/dlmalloc.h
  PATHS ${include_locations}
  ${LOFAR_SOURCE_DIR}
  NO_DEFAULT_PATH
  )

get_filename_component (SHMEM_INCLUDES ${SHMEM_INCLUDES} ABSOLUTE)

## -----------------------------------------------------------------------------
## Check for the library


find_library (SHMEM_LIBRARIES libshmem.a
  PATHS ${lib_locations}
  ${LOFAR_SOURCE_DIR}/installed/${BUILD_VARIANT}/lib64/
  NO_DEFAULT_PATH
  )

## -----------------------------------------------------------------------------
## Actions taken when all components have been found

if (SHMEM_INCLUDES AND SHMEM_LIBRARIES)
  set (SHMEM_FOUND TRUE)
else (SHMEM_INCLUDES AND SHMEM_LIBRARIES)
  set (SHMEM_FOUND FALSE)
  if (NOT SHMEM_FIND_QUIETLY)
    if (NOT SHMEM_INCLUDES)
      message (STATUS "Unable to find SHMEM header files!")
    endif (NOT SHMEM_INCLUDES)
    if (NOT SHMEM_LIBRARIES)
      message (STATUS "Unable to find SHMEM library files!")
    endif (NOT SHMEM_LIBRARIES)
  endif (NOT SHMEM_FIND_QUIETLY)
endif (SHMEM_INCLUDES AND SHMEM_LIBRARIES)

if (SHMEM_FOUND)
  if (NOT SHMEM_FIND_QUIETLY)
    message (STATUS "Found components for SHMEM")
    message (STATUS "SHMEM_INCLUDES  = ${SHMEM_INCLUDES}")
    message (STATUS "SHMEM_LIBRARIES = ${SHMEM_LIBRARIES}")
  endif (NOT SHMEM_FIND_QUIETLY)
else (SHMEM_FOUND)
  if (SHMEM_FIND_REQUIRED)
    message (FATAL_ERROR "Could not find SHMEM!")
  endif (SHMEM_FIND_REQUIRED)
endif (SHMEM_FOUND)

## -----------------------------------------------------------------------------
## Mark advanced variables

mark_as_advanced (
  SHMEM_INCLUDES
  SHMEM_LIBRARIES
  )
