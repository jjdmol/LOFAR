# $Id$

# List of header files that will be installed.
set(inst_HEADERS
  Converter.h
  ClassifConv.h
  ParamTypeConv.h
  TreeStateConv.h
  TreeTypeConv.h
  UnitConv.h
  OTDBnode.h
  OTDBparam.h
  OTDBtree.h
  OTDBtypes.h
  OTDBvalue.h
  TreeState.h
  VICnodeDef.h
  OTDBconstants.h
  misc.h
  OTDBconnection.h
  TreeMaintenance.h
  TreeValue.h
  CampaignInfo.h
  Campaign.h
  DefaultTemplate.h)
# Create symbolic link to include directory.
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink 
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/include/${PACKAGE_NAME})

# Install header files.
install(FILES ${inst_HEADERS} DESTINATION include/${PACKAGE_NAME})
