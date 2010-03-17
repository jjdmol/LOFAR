# $Id$

include(LofarCTest)

include_directories(${PACKAGE_SOURCE_DIR}/src) 

lofar_add_test(tCN_Processing tCN_Processing.cc)
lofar_add_test(tBeamForming tBeamForming.cc)
lofar_add_test(tPencilBeamFormer tPencilBeamFormer.cc)
lofar_add_test(tStokes tStokes.cc)
