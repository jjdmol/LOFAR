# $Id$

include(LofarCTest)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/tStorageProcesses.run2.in
  ${CMAKE_CURRENT_BINARY_DIR}/tStorageProcesses.run2 @ONLY)

# DummyStorage is started by tStorageProcesses to emulate an OutputProc process
lofar_add_executable(DummyStorage DummyStorage.cc)
lofar_add_test(tStorageProcesses tStorageProcesses.cc DEPENDS DummyStorage)

