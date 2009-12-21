# $Id$

include(LofarPackageVersion)

lofar_add_library(jotdb2 SHARED 
  Package__Version.cc
  nl_astron_lofar_sas_otb_jotdb2_jCommon.cc
  nl_astron_lofar_sas_otb_jotdb2_jOTDBconnection.cc
  nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance.cc
  nl_astron_lofar_sas_otb_jotdb2_jClassifConv.cc
  nl_astron_lofar_sas_otb_jotdb2_jParamTypeConv.cc
  nl_astron_lofar_sas_otb_jotdb2_jTreeStateConv.cc
  nl_astron_lofar_sas_otb_jotdb2_jTreeTypeConv.cc
  nl_astron_lofar_sas_otb_jotdb2_jUnitConv.cc
  nl_astron_lofar_sas_otb_jotdb2_jInitCPPLogger.cc
  nl_astron_lofar_sas_otb_jotdb2_jTreeValue.cc)

lofar_add_executable(versionjotdb2 versionjotdb2.cc)

