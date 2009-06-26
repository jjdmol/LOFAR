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

cmake_minimum_required(VERSION 2.6)

## ---------------------------------------------------------------------------
## Preamble: set compilers
## ---------------------------------------------------------------------------
include(LofarInit)

## ---------------------------------------------------------------------------
## Project characteristics
## ---------------------------------------------------------------------------
project(APL)
include(LofarGeneral)

## ---------------------------------------------------------------------------
## Create directory APL in the binary include directory; it will hold symbolic
## links to the different subcomponents of APL.
## ---------------------------------------------------------------------------
execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory
  ${CMAKE_BINARY_DIR}/include/APL)

## ---------------------------------------------------------------------------
## Subdirectories
## ---------------------------------------------------------------------------
lofar_add_subdirectory(APLCommon)
lofar_add_subdirectory(CEPCU)
lofar_add_subdirectory(CUDaemons)
lofar_add_subdirectory(CURTDBDaemons)
lofar_add_subdirectory(MainCU)
lofar_add_subdirectory(PAC)
lofar_add_subdirectory(PIC)
lofar_add_subdirectory(RTCCommon)
lofar_add_subdirectory(RTDBCommon)
lofar_add_subdirectory(StationCU)
lofar_add_subdirectory(TestCtlr)

## ---------------------------------------------------------------------------
## Generate configuration header file.
## ---------------------------------------------------------------------------
include(LofarConfig)

