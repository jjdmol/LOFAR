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

# Add MAC protocol(s)
include(MACAddProtocol)
mac_add_protocol(Echo_Protocol "${MACIO_SOURCE_DIR}/autogen")

# Add current binary directory to -I path, so that the generated C++ header
# files can be found.
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# Tests cannot be run at this stage, because they all need to connect to a
# Service Broker, which has not been built yet. Therefore we will only build
# the test programs.
lofar_add_executable(tAutoOpen tAutoOpen.cc tServer.cc Echo_Protocol.cc)
lofar_add_executable(tEventQueue tEventQueue.cc tClient.cc Echo_Protocol.cc)
lofar_add_executable(tStateResult tStateResult.cc tServer.cc Echo_Protocol.cc)
lofar_add_executable(tmEcho Echo_Protocol.cc Echo.cc)
lofar_add_executable(tmPing Echo_Protocol.cc Ping.cc)
lofar_add_executable(tGCFPort Echo_Protocol.cc tGCFPort.cc)
lofar_add_executable(tGCFTimer tTimer.cc)
lofar_add_executable(tGCFTask1 tGCFTask1.cc testTask.cc)
lofar_add_executable(tGCFTask2 tGCFTask2.cc testTask.cc)
lofar_add_executable(tGCFTask3 tGCFTask3.cc testTask.cc)
lofar_add_executable(tGCFTask4 tGCFTask4.cc testTask.cc)
lofar_add_executable(tGCFTask5 tGCFTask5.cc testTask.cc)
