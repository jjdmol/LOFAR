# $Id$

include(LofarPackageVersion)

# Create symbolic link to include directory.
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink 
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/include/${PACKAGE_NAME})

lofar_add_library(cointerface
  Package__Version.cc
  Allocator.cc
  CorrelatedData.cc
  BlockID.cc
  BudgetTimer.cc
  FinalMetaData.cc
  LTAFeedback.cc
  Stream.cc
  Parset.cc
  RunningStatistics.cc
  TABTranspose.cc
  TimeFuncs.cc
  RingCoordinates.cc
  SelfDestructTimer.cc
)

lofar_add_bin_program(versioncointerface versioncointerface.cc)

lofar_add_sbin_program(cmpfloat cmpfloat.cc)
lofar_add_sbin_program(createFeedback createFeedback.cc)
