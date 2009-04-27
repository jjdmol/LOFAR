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

include(LofarCTest)

set (CHECK_PROGRAMS
  testLogger
  tAllocator
  tDataConvert
  tFileLocator
  tStringUtil
  tStreamUtil
  tHexdump
#  testSocket  (SKIPPED)
  tComplex
#  tProcess    (SKIPPED)
  tRunOnNode
  tTimer
#  tNumeric    (SKIPPED)
  tSingleton
  tObjectFactory
#  tSystemUtil
#  tSaxpy
  tPrettyUnits
  tParameterValue
  tParameterSet
  tParameterSetPerf
  tBacktrace
)

add_executable(testLogger testLogger.cc)
add_executable(tAllocator tAllocator.cc)
add_executable(tDataConvert tDataConvert.cc)
add_executable(tFileLocator tFileLocator.cc)
add_executable(tStringUtil tStringUtil.cc)
add_executable(tStreamUtil tStreamUtil.cc)
add_executable(tHexdump tHexdump.cc)
#add_executable(testSocket testSocket.cc)
add_executable(tComplex tComplex.cc)
#add_executable(tProcess tProcess.cc)
add_executable(tRunOnNode tRunOnNode.cc)
add_executable(tTimer tTimer.cc)
#add_executable(tNumeric tNumeric.cc)
add_executable(tSingleton tSingleton.cc)
add_executable(tObjectFactory tObjectFactory.cc Shapes.cc Coordinates.cc)
#add_executable(tSystemUtil tSystemUtil.cc)
#add_executable(tSaxpy tSaxpy.cc)
add_executable(tPrettyUnits tPrettyUnits.cc)
add_executable(tParameterValue tParameterValue.cc)
add_executable(tParameterSet tParameterSet.cc)
add_executable(tParameterSetPerf tParameterSetPerf.cc)
add_executable(tBacktrace tBacktrace.cc)

foreach (prog ${CHECK_PROGRAMS})
  add_test(${prog} ${CMAKE_CURRENT_SOURCE_DIR}/${prog}.sh)
  add_dependencies(check ${prog})
  target_link_libraries(${prog} common ${LOFAR_LIBRARIES})
endforeach (prog CHECK_PROGRAMS)
