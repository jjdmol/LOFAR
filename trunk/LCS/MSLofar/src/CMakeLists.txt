# $Id$

include(LofarPackageVersion)

lofar_add_library(mslofar
  Package__Version.cc
  MSLofarTable.cc
  MSAntennaField.cc
  MSAntennaFieldColumns.cc
  MSElementFailure.cc
  MSElementFailureColumns.cc
  MSLofarAntenna.cc
  MSLofarAntennaColumns.cc
  MSLofarObservation.cc
  MSLofarObsColumns.cc
  MSStation.cc
  MSStationColumns.cc
  BeamTables.cc
)

lofar_add_bin_program(versionmslofar versionmslofar.cc)
lofar_add_bin_program(makebeamtables makebeamtables.cc)

