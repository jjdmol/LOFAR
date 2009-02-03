#  $Id$
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

## ---------------------------------------------------------------------------
## Project characteristics
## ---------------------------------------------------------------------------
project (LCS)
cmake_minimum_required (VERSION 2.6)

## ---------------------------------------------------------------------------
##  CMake configuration settings
## ---------------------------------------------------------------------------

# Include our local settings file
include(CMakeSettings)

## ---------------------------------------------------------------------------

include(CTest)

#set(lofar_sharedir "aap/noot/mies")
#set(ENV{lofar_sharedir} ${lofar_sharedir})

#set(CTEST_ENVIRONMENT
#  "lofar_sharedir=${lofar_sharedir}")

configure_file(${CMAKE_SOURCE_DIR}/config.h.cmake
               ${CMAKE_BINARY_DIR}/config.h)
configure_file(${CMAKE_SOURCE_DIR}/lofar_config.h.cmake
               ${CMAKE_BINARY_DIR}/lofar_config.h)

#configure_file(${CMAKE_CURRENT_SOURCE_DIR}/runtest.sh.cmake
#               ${CMAKE_CURRENT_BINARY_DIR}/runtest.sh)

include(FindBoost)
if(Boost_FOUND)
  add_definitions(-DHAVE_BOOST)
endif(Boost_FOUND)


add_subdirectory(Tools)
add_subdirectory(Common)
