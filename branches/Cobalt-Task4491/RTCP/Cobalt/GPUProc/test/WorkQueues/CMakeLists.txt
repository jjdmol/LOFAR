# $Id$

include(LofarCTest)

if(UNITTEST++_FOUND)
  lofar_add_test(tCorrelatorWorkQueue tCorrelatorWorkQueue.cc)
  lofar_add_test(tWorkQueue tWorkQueue.cc)
endif()

lofar_add_test(tCorrelatorWorkQueueProcessSb tCorrelatorWorkQueueProcessSb.cc)

# Trick tests that compile kernels that need to be installed into thinking
# that they can find the kernels using the LOFARROOT environment variable.
set_tests_properties(
  tCorrelatorWorkQueueProcessSb
  PROPERTIES ENVIRONMENT "LOFARROOT=${PACKAGE_SOURCE_DIR}"
)

