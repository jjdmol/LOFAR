# $Id$

include(LofarPackageVersion)

lofar_add_library(mwcontrol
  Package__Version.cc
  MWGlobalSpec.cc
  MWLocalSpec.cc
  MWMultiSpec.cc
  MWParameterHandler.cc
  MWSpec.cc
  MWStrategySpec.cc
  PredifferBBS.cc
  PredifferProxy.cc
  SolverBBS.cc
  SolverProxy.cc)

lofar_add_bin_program(versionmwcontrol versionmwcontrol.cc)
