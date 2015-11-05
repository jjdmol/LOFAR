# $Id$

include(LofarCTest)

# DummyStorage is started by tStorageProcesses to emulate an OutputProc process
lofar_add_executable(DummyStorage DummyStorage.cc)
lofar_add_test(tStorageProcesses tStorageProcesses.cc DEPENDS DummyStorage)

