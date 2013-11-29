# $Id$

include(LofarCTest)

lofar_add_test(tCorrelatorPipelineProcessObs tCorrelatorPipelineProcessObs.cc)

# Trick tests that compile kernels that need to be installed into thinking
# that they can find the kernels using the LOFARROOT environment variable.
set_tests_properties(
  tCorrelatorPipelineProcessObs
  PROPERTIES ENVIRONMENT "LOFARROOT=${PACKAGE_SOURCE_DIR}"
)

