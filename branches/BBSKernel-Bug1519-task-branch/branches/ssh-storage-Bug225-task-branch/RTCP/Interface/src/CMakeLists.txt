# $Id$

include(LofarPackageVersion)

lofar_add_library(interface
  Package__Version.cc
  Allocator.cc
  CN_Configuration.cc
  CN_Mapping.cc
  CN_ProcessingPlan.cc
  Parset.cc
  PencilCoordinates.cc
  RSPTimeStamp.cc)

lofar_add_bin_program(versioninterface versioninterface.cc)
