# $Id$

include(LofarCTest)

lofar_add_test(tClusterDesc tClusterDesc.cc)
lofar_add_test(tNodeDesc tNodeDesc.cc)
lofar_add_test(tVdsDesc tVdsDesc.cc)
lofar_add_test(tVdsPartDesc tVdsPartDesc.cc)
lofar_add_test(tWorkersDesc tWorkersDesc.cc)
lofar_add_test(tSocketConnection tSocketConnection.cc)
lofar_add_test(tfinddproc DEPENDS finddproc)
lofar_add_test(tstartdproc DEPENDS tstartdproc.cc)
lofar_add_test(trundist)
