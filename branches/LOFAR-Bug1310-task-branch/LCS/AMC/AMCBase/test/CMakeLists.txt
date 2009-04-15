include(LofarCTest)

set(CHECK_PROGRAMS
  tCoord3D
  tDirection
  tPosition
  tEpoch
  tConverterCommand
  tConverterStatus
  tBlobIO
  tDH_Request
  tDH_Result)

foreach(prog ${CHECK_PROGRAMS})
  add_executable(${prog} ${prog}.cc)
  add_test(${prog} ${CMAKE_CURRENT_SOURCE_DIR}/${prog}.sh)
  add_dependencies(check ${prog})
  target_link_libraries(${prog} amcbase ${LOFAR_LIBRARIES})
endforeach(prog ${CHECK_PROGRAMS})
