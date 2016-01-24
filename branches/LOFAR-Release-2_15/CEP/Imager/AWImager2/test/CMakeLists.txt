# $Id$

include(LofarCTest)

lofar_add_test(tDirtyImage2)
#lofar_add_test(tClean2)
if (IDG_FOUND)
  lofar_add_test(tFTMachineIDG)
endif()


