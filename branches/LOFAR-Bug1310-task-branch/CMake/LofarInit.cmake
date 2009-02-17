#  LofarInit.cmake: 
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

if(NOT DEFINED LOFAR_INIT_INCLUDED)
set(LOFAR_INIT_INCLUDED TRUE CACHE BOOL "LOFAR_INIT_INCLUDED" FORCE)

message(STATUS "**** ENTER: LofarInit.cmake ****")

include(LofarVariants)

## ----------------------------------------------------------------------------
## Find compiler and variant.
## ----------------------------------------------------------------------------

# Compiler and variant are derived from the name of the binary directory.
get_filename_component(cmpvar ${CMAKE_BINARY_DIR} NAME)
string(TOUPPER ${cmpvar} cmpvar)
string(REGEX REPLACE "\(.*)_.*" "\\1" cmp ${cmpvar})
string(REGEX REPLACE ".*_\(.*)" "\\1" var ${cmpvar})

set(LOFAR_COMPILER_SUITE ${cmp} CACHE INTERNAL "Compiler suite (e.g., gnu)")
set(LOFAR_BUILD_VARIANT ${var} CACHE INTERNAL "Build variant (e.g., debug)")

# Get the list of compilers for this compiler suite.
set(compilers "${LOFAR_COMPILER_SUITE}_COMPILERS")
if(NOT DEFINED ${compilers})
  message(FATAL_ERROR 
    "${compilers} is not defined. Check your variants file!")
endif(NOT DEFINED ${compilers})

foreach(cmp ${${compilers}})
  string(REGEX REPLACE "${LOFAR_COMPILER_SUITE}_" "" lang ${cmp})
  message(STATUS "${lang} compiler: ${${cmp}}")
  set(CMAKE_${lang}_COMPILER ${${cmp}} CACHE FILEPATH "${lang} compiler" FORCE)
endforeach(cmp ${compilers})

message(STATUS "**** LEAVE: LofarInit.cmake ****")

endif(NOT DEFINED LOFAR_INIT_INCLUDED)
