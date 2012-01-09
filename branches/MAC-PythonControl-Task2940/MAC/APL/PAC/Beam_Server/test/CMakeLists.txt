# $Id$

include(LofarCTest)

install(PROGRAMS
  beamctltest
  DESTINATION bin)

foreach(file AVTStub.conf EPATest.conf SweepTest.conf BeamServerTest.conf)
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/${file}.in 
    ${CMAKE_CURRENT_BINARY_DIR}/${file})
  install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/${file}
    DESTINATION etc)
endforeach(file)
