# $Id$

include(LofarPackageVersion)

# Add MAC protocols (generates a .ph and .cc file from a .prot file)
include(MACAddProtocol)
mac_add_protocol(TP_Protocol "${MACIO_SOURCE_DIR}/autogen")

# Add the directory where the generated C++ headers are stored to the -I path.
include_directories(${CMAKE_BINARY_DIR}/include/MAC)

# Create symbolic link in binary include directory to current directory.
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/include/${PACKAGE_NAME})

# Add current source directory to -I path.
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

lofar_add_library(tp_protocol
  TP_Protocol.cc
  RawEvent.cc)

lofar_add_library(tbbdriver
  Package__Version.cc
  TP_Protocol.ph
  DriverSettings.cc
  Command.cc
  AllocCmd.cc
  BoardCmdHandler.cc
  MsgHandler.cc
  FreeCmd.cc
  RecordCmd.cc
  StopCmd.cc
  TrigReleaseCmd.cc
  TrigGenCmd.cc
  TrigSetupCmd.cc
  TrigSetupSameCmd.cc
  TrigCoefCmd.cc
  TrigCoefSameCmd.cc
  TrigInfoCmd.cc
  ReadCmd.cc
  UdpCmd.cc
  PageperiodCmd.cc
  VersionCmd.cc
  SizeCmd.cc
  StatusCmd.cc
  ClearCmd.cc
  ResetCmd.cc
  ConfigCmd.cc
  ErasefCmd.cc
  ReadfCmd.cc
  WritefCmd.cc
  ImageInfoCmd.cc
  ReadwCmd.cc
  WritewCmd.cc
  ReadrCmd.cc
  WriterCmd.cc
  ReadxCmd.cc
  ArpCmd.cc
  ArpModeCmd.cc
  StopCepCmd.cc
  CepStatusCmd.cc
  CepDelayCmd.cc
  WatchDogCmd.cc
  TempLimitCmd.cc)

lofar_add_bin_program(versiontbb_driver versiontbb_driver.cc)
lofar_add_bin_program(tbbctl tbbctl.cc)
lofar_add_bin_program(TBBDriver TBBDriver.cc)

