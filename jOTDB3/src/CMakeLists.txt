# $Id: CMakeLists.txt 15084 2010-02-28 09:20:06Z coolen $

include(LofarPackageVersion)

lofar_add_library(jotdb3 SHARED 
  Package__Version.cc
  nl_astron_lofar_sas_otb_jotdb3_jCommon.cc
  nl_astron_lofar_sas_otb_jotdb3_jCampaign.cc
  nl_astron_lofar_sas_otb_jotdb3_jClassifConv.cc
  nl_astron_lofar_sas_otb_jotdb3_jInitCPPLogger.cc
  nl_astron_lofar_sas_otb_jotdb3_jOTDBconnection.cc
  nl_astron_lofar_sas_otb_jotdb3_jParamTypeConv.cc
  nl_astron_lofar_sas_otb_jotdb3_jTreeMaintenance.cc
  nl_astron_lofar_sas_otb_jotdb3_jTreeStateConv.cc
  nl_astron_lofar_sas_otb_jotdb3_jTreeTypeConv.cc
  nl_astron_lofar_sas_otb_jotdb3_jTreeValue.cc
  nl_astron_lofar_sas_otb_jotdb3_jUnitConv.cc
)
lofar_add_bin_program(versionjotdb3 versionjotdb3.cc)

