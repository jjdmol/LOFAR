# $Id: CMakeLists.txt 25878 2013-08-01 02:08:38Z amesfoort $


install(FILES
  parset-additions.d/default/HardwareList.parset
  parset-additions.d/default/HardwareUsed.parset
  parset-additions.d/default/FinalMetaDataGatherer.parset
  parset-additions.d/default/MAC-feedback.parset
  parset-additions.d/default/OutputProc.parset
  parset-additions.d/default/StationCalibration.parset
  parset-additions.d/default/StationStreams.parset
  DESTINATION etc/parset-additions.d/default)

INSTALL(CODE "FILE(MAKE_DIRECTORY \${ENV}\${CMAKE_INSTALL_PREFIX}etc/parset-additions.d/override)")
