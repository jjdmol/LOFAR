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

message(STATUS "**** ENTER: LofarConf.cmake ****")

## ----------------------------------------------------------------------------
## Includes
## ----------------------------------------------------------------------------

include(LofarInit)
include(LofarGeneral)
include(LofarOptions)


message(STATUS "[LofarConf] CMAKE_C_COMPILER = ${CMAKE_C_COMPILER}")
message(STATUS "[LofarConf] CMAKE_CXX_COMPILER = ${CMAKE_CXX_COMPILER}")
message(STATUS "[LofarConf] CMAKE_FORTRAN_COMPILER = ${CMAKE_FORTRAN_COMPILER}")
message(STATUS "[LofarConf] CMAKE_CXX_FLAGS_DEBUG = ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "[LofarConf] CMAKE_CXX_FLAGS_RELEASE = ${CMAKE_CXX_FLAGS_RELEASE}")

message(STATUS "**** LEAVE: LofarConf.cmake ****")
