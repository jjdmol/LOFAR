include(LofarCTest)

include_directories(${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../include)

set (CHECKTOOLPROGS
  testLogger
  tAllocator
  tDataConvert
  tFileLocator
  tStringUtil
  tStreamUtil
  tHexdump
  testSocket
  tComplex
  tProcess
  tRunOnNode
  tTimer
  tNumeric
  tSingleton
  tObjectFactory
#  tSystemUtil
#  tSaxpy
  tPrettyUnits
  tParameterValue
  tParameterSet
  tParameterSetPerf
#  tBacktrace
)

message(STATUS "[LCS/Common/test] LOFAR_LIBRARIES = ${LOFAR_LIBRARIES}")

add_executable(testLogger testLogger.cc)
add_executable(tAllocator tAllocator.cc)
add_executable(tDataConvert tDataConvert.cc)
add_executable(tFileLocator tFileLocator.cc)
add_executable(tStringUtil tStringUtil.cc)
add_executable(tStreamUtil tStreamUtil.cc)
add_executable(tHexdump tHexdump.cc)
add_executable(testSocket testSocket.cc)
add_executable(tComplex tComplex.cc)
add_executable(tProcess tProcess.cc)
add_executable(tRunOnNode tRunOnNode.cc)
add_executable(tTimer tTimer.cc)
add_executable(tNumeric tNumeric.cc)
add_executable(tSingleton tSingleton.cc)
add_executable(tObjectFactory tObjectFactory.cc Shapes.cc Coordinates.cc)
#add_executable(tSystemUtil tSystemUtil.cc)
#add_executable(tSaxpy tSaxpy.cc)
add_executable(tPrettyUnits tPrettyUnits.cc)
add_executable(tParameterValue tParameterValue.cc)
add_executable(tParameterSet tParameterSet.cc)
add_executable(tParameterSetPerf tParameterSetPerf.cc)
#add_executable(tBacktrace tBacktrace.cc)

foreach (prog ${CHECKTOOLPROGS})
  add_test(${prog} ${CMAKE_CURRENT_SOURCE_DIR}/${prog}.sh)
  target_link_libraries(${prog} common ${LOFAR_LIBRARIES})
endforeach (prog CHECKTOOLPROGS)
