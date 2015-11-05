# $Id$

include(LofarCTest)

lofar_add_test(tClusterDesc tClusterDesc.cc)
lofar_add_test(tNodeDesc tNodeDesc.cc)
lofar_add_test(tVdsDesc tVdsDesc.cc)
lofar_add_test(tVdsPartDesc tVdsPartDesc.cc)
lofar_add_test(tWorkersDesc tWorkersDesc.cc)
lofar_add_test(tSocketConnection tSocketConnection.cc)
lofar_add_test(tSocketConnectionSet tSocketConnectionSet.cc)
lofar_add_test(tfinddproc DEPENDS finddproc)
lofar_add_test(tstartdproc tstartdproc.cc)
lofar_add_test(trundist)

# The SocketConnection tests must be run serially
if(BUILD_TESTING)
  set_tests_properties(
    tSocketConnection
    tSocketConnectionSet
    PROPERTIES RUN_SERIAL ON)
endif(BUILD_TESTING)
