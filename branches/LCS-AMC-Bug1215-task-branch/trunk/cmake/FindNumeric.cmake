# +-----------------------------------------------------------------------------+
# | $Id:: IO.h 393 2007-06-13 10:49:08Z baehren                               $ |
# +-----------------------------------------------------------------------------+
# |   Copyright (C) 2007                                                        |
# |   Lars B"ahren (bahren@astron.nl)                                           |
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

# - Check for the presence of Numeric
#
# The following variables are set when Numeric is found:
#  HAVE_NUMERIC       = Set to true, if all components of Numeric
#                          have been found.
#  NUMERIC_INCLUDES   = Include path for the header files of Numeric
#  NUMERIC_LIBRARIES  = Link these to use Numeric
#  NUMERIC_LFGLAS     = Linker flags (optional)

## -----------------------------------------------------------------------------
## Search locations

include (CMakeSettings)

## -----------------------------------------------------------------------------
## Check for the header files

find_path (NUMERIC_INCLUDES arrayobject.h
  PATHS ${include_locations}
  PATH_SUFFIXES
  python
  python/numeric
  python/Numeric
  NO_DEFAULT_PATH
  )

## most likely we need to adjust the path in order to support include via
## something like <numeric/numeric.h>

string (REGEX REPLACE "include/python/numeric" "include/python" NUMERIC_INCLUDES ${NUMERIC_INCLUDES})

string (REGEX REPLACE "include/python/Numeric" "include/python" NUMERIC_INCLUDES ${NUMERIC_INCLUDES})

## -----------------------------------------------------------------------------
## Check for the library

find_library (NUMERIC_LIBRARIES _numpy
  PATHS ${lib_locations}
  PATH_SUFFIXES
  python
  python/numeric
  python/Numeric
  python/site-packages/Numeric
  NO_DEFAULT_PATH
  )

## -----------------------------------------------------------------------------
## Actions taken when all components have been found

if (NUMERIC_INCLUDES AND NUMERIC_LIBRARIES)
  set (HAVE_NUMERIC TRUE)
else (NUMERIC_INCLUDES AND NUMERIC_LIBRARIES)
  set (HAVE_NUMERIC FALSE)
  if (NOT NUMERIC_FIND_QUIETLY)
    if (NOT NUMERIC_INCLUDES)
      message (STATUS "Unable to find NUMERIC header files!")
    endif (NOT NUMERIC_INCLUDES)
    if (NOT NUMERIC_LIBRARIES)
      message (STATUS "Unable to find NUMERIC library files!")
    endif (NOT NUMERIC_LIBRARIES)
  endif (NOT NUMERIC_FIND_QUIETLY)
endif (NUMERIC_INCLUDES AND NUMERIC_LIBRARIES)

if (HAVE_NUMERIC)
  if (NOT NUMERIC_FIND_QUIETLY)
    message (STATUS "Found components for NUMERIC")
    message (STATUS "NUMERIC_INCLUDES  = ${NUMERIC_INCLUDES}")
    message (STATUS "NUMERIC_LIBRARIES = ${NUMERIC_LIBRARIES}")
  endif (NOT NUMERIC_FIND_QUIETLY)
else (HAVE_NUMERIC)
  if (NUMERIC_FIND_REQUIRED)
    message (FATAL_ERROR "Could not find NUMERIC!")
  endif (NUMERIC_FIND_REQUIRED)
endif (HAVE_NUMERIC)

## -----------------------------------------------------------------------------
## Mark advanced variables

mark_as_advanced (
  NUMERIC_INCLUDES
  NUMERIC_LIBRARIES
  )
