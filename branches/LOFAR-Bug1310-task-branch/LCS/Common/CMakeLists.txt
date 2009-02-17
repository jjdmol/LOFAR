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
project(Common)
cmake_minimum_required(VERSION 2.6)

## ---------------------------------------------------------------------------
##  CMake configuration settings
## ---------------------------------------------------------------------------

include(CMakeSettings)
include(CTest)

include(CheckTypeSize)
check_type_size("ushort" HAVE_USHORT)
check_type_size("uint" HAVE_UINT)
check_type_size("ulong" HAVE_ULONG)
check_type_size("long long" HAVE_LONG_LONG)

add_definitions(-DHAVE_CONFIG_H)
add_definitions(-DHAVE_BOOST)
add_definitions(-DHAVE_LOG4CPLUS)
add_definitions(-I/opt/lofar/external/log4cplus/include)
link_directories(/opt/lofar/external/log4cplus/lib64)

## ---------------------------------------------------------------------------
## Subdirectories to add
## ---------------------------------------------------------------------------
add_subdirectory(include/Common)
add_subdirectory(src)
if(BUILD_TESTING)
  add_subdirectory(test)
endif(BUILD_TESTING)
