# $Id$

include(LofarCTest)

# Add the directory where the generated C++ headers are stored to the -I path.
include_directories(${PACKAGE_BINARY_DIR}/src)

# Add the project's source directory to the -I path.
include_directories(${PACKAGE_SOURCE_DIR}/src)

#lofar_add_library(tbbstub StubRawEvent.cc)
lofar_add_sbin_program(TPStub TPStub.cc)
lofar_add_sbin_program(TBBTest TBBTest.cc)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/TPStub.conf.in
  ${CMAKE_CURRENT_BINARY_DIR}/TPStub.conf)
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/TBBTest.conf.in
  ${CMAKE_CURRENT_BINARY_DIR}/TBBTest.conf)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/TPStub.conf
  ${CMAKE_CURRENT_BINARY_DIR}/TBBTest.conf
  DESTINATION etc)
