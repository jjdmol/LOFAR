# $Id$

include(LofarCTest)

# Add project's source directory to -I path.
include_directories(${PACKAGE_SOURCE_DIR}/src)

lofar_add_test(tCableAttenuation 
  tCableAttenuation.cc
  ${PACKAGE_SOURCE_DIR}/src/CableAttenuation.cc)
  
lofar_add_test(tRCUCables
  tRCUCables.cc
  ${PACKAGE_SOURCE_DIR}/src/RCUCables.cc
  ${PACKAGE_SOURCE_DIR}/src/CableAttenuation.cc)

lofar_add_sbin_program(EPAStub EPAStub.cc)
lofar_add_sbin_program(RSPTest RSPTest.cc)

install(PROGRAMS
  beamtest
  DESTINATION sbin)

foreach(file RSPTest.conf EPAStub.conf EPAStub.log_prop)
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/${file}.in
    ${CMAKE_CURRENT_BINARY_DIR}/${file})
  install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/${file}
    DESTINATION etc)
endforeach(file)
