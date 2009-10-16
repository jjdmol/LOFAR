# $Id$

# List of header files that will be installed.
set(inst_HEADERS
  nl_astron_lofar_sas_otb_jotdb2_jClassifConv.h
  nl_astron_lofar_sas_otb_jotdb2_jCommon.h
  nl_astron_lofar_sas_otb_jotdb2_jInitCPPLogger.h
  nl_astron_lofar_sas_otb_jotdb2_jOTDBconnection.h
  nl_astron_lofar_sas_otb_jotdb2_jParamTypeConv.h
  nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance.h
  nl_astron_lofar_sas_otb_jotdb2_jTreeStateConv.h
  nl_astron_lofar_sas_otb_jotdb2_jTreeTypeConv.h
  nl_astron_lofar_sas_otb_jotdb2_jTreeValue.h
  nl_astron_lofar_sas_otb_jotdb2_jUnitConv.h)

# Create symbolic link to include directory.
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink 
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/include/${PACKAGE_NAME})

# Install header files.
install(FILES ${inst_HEADERS} DESTINATION include/${PACKAGE_NAME})
