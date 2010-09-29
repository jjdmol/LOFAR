# $Id$

include(LofarPackageVersion)

# Create symbolic link in binary include directory to current directory.
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/include/${PACKAGE_NAME})

# Add current source directory to -I path.
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

lofar_add_library(epa
  Package__Version.cc
  RawEvent.cc)

lofar_add_sbin_program(geni2c geni2c.cc)
lofar_add_bin_program(versionrsp_driver versionrsp_driver.cc)
lofar_add_bin_program(rspctl rspctl.cc)
lofar_add_bin_program(RSPDriver
  RSPDriver.cc
  Scheduler.cc
  Sequencer.cc
  Cache.cc
  StationSettings.cc
  RCUCables.cc
  CableAttenuation.cc
  CableSettings.cc
  SerdesBuffer.cc
  SetWeightsCmd.cc
  GetWeightsCmd.cc
  SetSubbandsCmd.cc
  GetSubbandsCmd.cc
  UpdSubbandsCmd.cc
  GetStatusCmd.cc
  UpdStatusCmd.cc
  SetRCUCmd.cc
  GetRCUCmd.cc
  UpdRCUCmd.cc
  SetHBACmd.cc
  GetHBACmd.cc
  ReadHBACmd.cc
  UpdHBACmd.cc
  SetRSUCmd.cc
  SetWGCmd.cc
  GetWGCmd.cc
  GetVersionsCmd.cc
  GetStatsCmd.cc
  UpdStatsCmd.cc
  GetXCStatsCmd.cc
  UpdXCStatsCmd.cc
  SetClocksCmd.cc
  GetClocksCmd.cc
  UpdClocksCmd.cc
  GetTDStatusCmd.cc
  UpdTDStatusCmd.cc
  GetRegisterStateCmd.cc
  UpdRegisterStateCmd.cc
  SetTBBCmd.cc
  GetTBBCmd.cc
  SetBypassCmd.cc
  GetBypassCmd.cc
  GetSPUStatusCmd.cc
  SetRawBlockCmd.cc
  GetRawBlockCmd.cc
  SetSplitterCmd.cc
  GetSplitterCmd.cc
  GetLatencyCmd.cc
  SetDatastreamCmd.cc
  GetDatastreamCmd.cc
  SyncAction.cc
  WriteReg.cc
  ReadReg.cc
  XWWrite.cc
  BWWrite.cc
  BWRead.cc
  SSWrite.cc
  SSRead.cc
  RCUWrite.cc
  RCUProtocolWrite.cc
  RCUResultRead.cc
  RCURead.cc
  RawBlockRead.cc
  RawBlockWrite.cc
  StatusRead.cc
  SstRead.cc
  BstRead.cc
  XstRead.cc
  BypassRead.cc
  VersionsRead.cc
  WGWrite.cc
  WGRead.cc
  CDOWrite.cc
  BSWrite.cc
  RSUWrite.cc
  TDSProtocolWrite.cc
  TDSResultWrite.cc
  TDSResultRead.cc
  TDSStatusWrite.cc
  TDSStatusRead.cc
  HBAProtocolWrite.cc
  HBAResultRead.cc
  RADWrite.cc
  TimestampWrite.cc
  TBBSettingsWrite.cc
  TBBBandselWrite.cc
  BypassWrite.cc
  SerdesWrite.cc
  SerdesRead.cc
  UpdSplitterCmd.cc
  LatencyRead.cc)
 
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/rspctl.conf.in
  ${CMAKE_CURRENT_BINARY_DIR}/rspctl.conf)
  
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/rspctl.conf
  DESTINATION etc)
