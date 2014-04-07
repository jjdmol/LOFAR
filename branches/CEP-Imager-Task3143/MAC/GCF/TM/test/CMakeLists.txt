# $Id$

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
#lofar_add_executable(tGCFPort Echo_Protocol.cc tGCFPort.cc)
lofar_add_executable(tITCPort Echo_Protocol.cc tITCPort.cc)
lofar_add_executable(tGCFTimer tTimer.cc)
lofar_add_executable(tGCFTask1 tGCFTask1.cc testTask.cc)
lofar_add_executable(tGCFTask2 tGCFTask2.cc testTask.cc)
lofar_add_executable(tGCFTask3 tGCFTask3.cc testTask.cc)
lofar_add_executable(tGCFTask4 tGCFTask4.cc testTask.cc)
lofar_add_executable(tGCFTask5 tGCFTask5.cc testTask.cc)

add_dependencies(tAutoOpen GCFTM-Echo_Protocol)
add_dependencies(tEventQueue GCFTM-Echo_Protocol)
add_dependencies(tStateResult GCFTM-Echo_Protocol)
add_dependencies(tmEcho GCFTM-Echo_Protocol)
add_dependencies(tmPing GCFTM-Echo_Protocol)
#add_dependencies(tGCFPort GCFTM-Echo_Protocol)
add_dependencies(tITCPort GCFTM-Echo_Protocol)
