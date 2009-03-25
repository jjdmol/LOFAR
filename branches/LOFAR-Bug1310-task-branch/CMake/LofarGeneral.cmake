#  LofarGeneral.cmake: 
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

message(STATUS "**** ENTER: LofarGeneral.cmake ****")

include(LofarInit)

## ----------------------------------------------------------------------------
## Check for typedefs of primitive types
## ----------------------------------------------------------------------------
include(CheckTypeSize)
check_type_size("ushort"    HAVE_USHORT   )
check_type_size("uint"      HAVE_UINT     )
check_type_size("ulong"     HAVE_ULONG    )
check_type_size("long long" HAVE_LONG_LONG)

## ----------------------------------------------------------------------------
## Define `AUTO_FUNCTION_NAME' as either __PRETTY_FUNCTION__, __FUNCTION__,
## or "<unknown>", depending on compiler support for function name macro.
## ----------------------------------------------------------------------------
include(CheckCSourceCompiles)
foreach(func_name __PRETTY_FUNCTION__ __FUNCTION__ "\"<unkown>\"")
  check_c_source_compiles("
    #include <stdio.h> 
    int main() { puts(${func_name}); }
    " HAVE_${func_name})
  if(HAVE_${func_name})
    set(AUTO_FUNCTION_NAME ${func_name} CACHE INTERNAL 
      "Define as __PRETTY_FUNCTION__, __FUNCTION__, or \"<unknown>\"")
    break()
  endif(HAVE_${func_name})
endforeach(func_name)

## ----------------------------------------------------------------------------
## Locate the Doxygen documentation tool
## ----------------------------------------------------------------------------
find_package(Doxygen)

## ----------------------------------------------------------------------------
## Initialize the LOFAR logger
## ----------------------------------------------------------------------------
include(LofarFindPackage)
include(LofarLogger)
lofar_logger()
message(STATUS "HAVE_LOG4CPLUS = ${HAVE_LOG4CPLUS}")
#lofar_find_package(pqxx 1 pqxx/pqxx)
#lofar_DEBUG_OPTIMIZE([])
#lofar_FUNCTION_NAME([])
#lofar_BACKTRACE([])
#lofar_CHECK_INSTALL_IF_MODIFIED([])
#lofar_QATOOLS([])
#lofar_DOCXX([])
#lofar_LOGGER([])

message(STATUS "**** LEAVE: LofarGeneral.cmake ****")
