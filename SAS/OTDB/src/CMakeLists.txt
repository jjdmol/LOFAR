# $Id$

include(LofarPackageVersion)

lofar_add_library(otdb
  Package__Version.cc
  OTDBconnection.cc
  misc.cc
  OTDBtree.cc
  OTDBnode.cc
  OTDBvalue.cc
  OTDBparam.cc
  VICnodeDef.cc
  Converter.cc
  wSpaceSplit.cc
  TreeMaintenance.cc
  TreeState.cc
  TreeValue.cc
  loadCompFile.cc
  CampaignInfo.cc
  Campaign.cc
  DefaultTemplate.cc)

lofar_add_bin_program(versionotdb versionotdb.cc)
lofar_add_bin_program(setStatus setStatus.cc)

install(FILES setStatus.conf DESTINATION etc)
