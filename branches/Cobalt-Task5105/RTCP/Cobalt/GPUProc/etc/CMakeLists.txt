# $Id: CMakeLists.txt 25878 2013-08-01 02:08:38Z amesfoort $


install(FILES
  parset-additions.d/HardwareList.parset
  parset-additions.d/HardwareUsed.parset
  parset-additions.d/FinalMetaDataGatherer.parset
  parset-additions.d/MAC-feedback.parset
  parset-additions.d/OutputProc.parset
  parset-additions.d/StationCalibration.parset
  parset-additions.d/StationStreams.parset
  DESTINATION etc/parset-additions.d)

