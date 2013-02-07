# $Id$

include(LofarCTest)

lofar_add_test(tCampaign tCampaign.cc)
lofar_add_test(tPICtree tPICtree.cc)
lofar_add_test(tPICvalue tPICvalue.cc)
lofar_add_executable(tBrokenHardware tBrokenHardware.cc)
lofar_add_test(tQueryPIC tQueryPIC.cc)
lofar_add_test(tVICcomp tVICcomp.cc)
lofar_add_test(tVTtree tVTtree.cc)
lofar_add_test(tVHtree tVHtree.cc)
lofar_add_test(tVHvalue tVHvalue.cc)
lofar_add_test(tMetadata tMetadata.cc)
lofar_add_test(tConnection tConnection.cc)
lofar_add_test(tParamTypeConv tParamTypeConv.cc)
