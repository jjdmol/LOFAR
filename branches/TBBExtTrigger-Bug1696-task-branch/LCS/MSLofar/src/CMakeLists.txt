# $Id$

include(LofarPackageVersion)

lofar_add_library(mslofar
  Package__Version.cc
  MSLofarTable.cc
  MSAntennaField.cc
  MSAntennaFieldColumns.cc
  MSElementFailure.cc
  MSElementFailureColumns.cc
  MSStation.cc
  MSStationColumns.cc
  MSLofarAntenna.cc
  MSLofarAntennaColumns.cc
  MSLofarField.cc
  MSLofarFieldColumns.cc
  MSLofarObservation.cc
  MSLofarObsColumns.cc
  MSLofar.cc
#  MeasurementSet.cc
  BeamTables.cc
)

lofar_add_bin_program(versionmslofar versionmslofar.cc)
lofar_add_bin_program(makebeamtables makebeamtables.cc)

