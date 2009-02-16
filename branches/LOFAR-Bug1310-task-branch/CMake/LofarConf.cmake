#  LofarConf.cmake: global configurations for LOFAR builds.
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

## ----------------------------------------------------------------------------
## Includes
## ----------------------------------------------------------------------------

include(LofarVariants)


## ----------------------------------------------------------------------------
## Find compiler and variant.
## ----------------------------------------------------------------------------

get_filename_component(cmpvar ${CMAKE_CURRENT_BINARY_DIR} NAME)
string(TOUPPER ${cmpvar} cmpvar)
string(REGEX REPLACE "\(.*)_.*" "\\1" compiler_suite ${cmpvar})
string(REGEX REPLACE ".*_\(.*)" "\\1" var ${cmpvar})

# Search for a list of compilers in `compiler_suite'.
set(compilers "${compiler_suite}_COMPILERS")
set(lofar_variant "${var}_VARIANT")

if(NOT DEFINED ${compilers})
  message(FATAL_ERROR 
    "${compilers} is not defined. Check your variants file!")
endif(NOT DEFINED ${compilers})

foreach(cmp ${${compilers}})
  string(REGEX REPLACE "${compiler_suite}_" "" lang ${cmp})
  message(STATUS "${lang} compiler: ${${cmp}}")
  set(CMAKE_${lang}_COMPILER ${${cmp}} CACHE FILEPATH "${lang} compiler" FORCE)
endforeach(cmp ${compilers})


message(STATUS ">>${lofar_compiler}<<")
message(STATUS ">>${lofar_variant}<<")

#set(CMAKE_CXX_COMPILER ${${cmp}_COMPILER} CACHE FILEPATH "C++ compiler" FORCE)

message(STATUS "[LofarConf] CMAKE_C_COMPILER = ${CMAKE_C_COMPILER}")
message(STATUS "[LofarConf] CMAKE_CXX_COMPILER = ${CMAKE_CXX_COMPILER}")
message(STATUS "[LofarConf] CMAKE_FORTRAN_COMPILER = ${CMAKE_FORTRAN_COMPILER}")
message(STATUS "[LofarConf] CMAKE_CXX_FLAGS_DEBUG = ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "[LofarConf] CMAKE_CXX_FLAGS_RELEASE = ${CMAKE_CXX_FLAGS_RELEASE}")

