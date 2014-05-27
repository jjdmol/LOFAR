# $Id$

include(LofarCTest)

lofar_add_test(tBestEffortQueue tBestEffortQueue.cc)
lofar_add_test(tCorrelatedData tCorrelatedData.cc)
lofar_add_test(tMultiDimArray tMultiDimArray.cc)
lofar_add_test(tgcd_lcm tgcd_lcm.cc)
lofar_add_test(tpow2 tpow2.cc)
lofar_add_test(tSparseSet tSparseSet.cc)
lofar_add_test(tfpequals tfpequals.cc)
lofar_add_test(tcmpfloat DEPENDS cmpfloat)


if(UNITTEST++_FOUND)
  lofar_add_test(tOMPThread tOMPThread.cc)
  lofar_add_test(tParset tParset.cc)
  lofar_add_test(tQueue2 tQueue2.cc)
  lofar_add_test(tRunningStatistics tRunningStatistics.cc)
  lofar_add_test(tTABTranspose tTABTranspose.cc)
  lofar_add_test(tRingCoordinates tRingCoordinates.cc)
endif(UNITTEST++_FOUND)
