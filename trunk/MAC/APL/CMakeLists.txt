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
lofar_add_subdirectory(APLCommon)     # Common tasks and functions
lofar_add_subdirectory(RTCCommon)     # Real Time Control functions
lofar_add_subdirectory(RTDBCommon)    # RealTimeDataBase common software
lofar_add_subdirectory(TestSuite)     # 
lofar_add_subdirectory(CUDaemons)     # Control Unit Daemons
lofar_add_subdirectory(CURTDBDaemons) # Control Unit RTDB Daemons
lofar_add_subdirectory(PIC)           # Physical Instrument Components (?)
lofar_add_subdirectory(PAC)           # ???
lofar_add_subdirectory(VHECR)         # VHECR related code
lofar_add_subdirectory(CEPCU)         # CEP-related MAC controllers
lofar_add_subdirectory(MainCU)        # Main Control Unit
lofar_add_subdirectory(StationCU)     # Station Control Unit software
lofar_add_subdirectory(TestCtlr)      # Testprograms for MAC controllers

## ---------------------------------------------------------------------------
## Generate configuration header file.
## ---------------------------------------------------------------------------
include(LofarConfig)

