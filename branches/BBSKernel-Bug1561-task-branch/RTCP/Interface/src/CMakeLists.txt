# $Id$

include(LofarPackageVersion)

lofar_add_library(interface
  Package__Version.cc
  Allocator.cc
  BeamCoordinates.cc
  CN_Mapping.cc
  DataFactory.cc
  Stream.cc
  Parset.cc
  RSPTimeStamp.cc)

lofar_add_bin_program(versioninterface versioninterface.cc)
