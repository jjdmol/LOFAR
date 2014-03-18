# $Id$

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/parset-additions.d/default/OutputProc.parset.in"
  "${CMAKE_CURRENT_BINARY_DIR}/parset-additions.d/default/OutputProc.parset"
  @ONLY
)

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/parset-additions.d/default/FinalMetaDataGatherer.parset.in"
  "${CMAKE_CURRENT_BINARY_DIR}/parset-additions.d/default/FinalMetaDataGatherer.parset"
  @ONLY
)

install(FILES
  parset-additions.d/default/HardwareList.parset
  parset-additions.d/default/HardwareUsed.parset
  ${CMAKE_CURRENT_BINARY_DIR}/parset-additions.d/default/FinalMetaDataGatherer.parset
  parset-additions.d/default/MAC-feedback.parset
  parset-additions.d/default/NoCascadingFFT.parset
  ${CMAKE_CURRENT_BINARY_DIR}/parset-additions.d/default/OutputProc.parset
  parset-additions.d/default/StationCalibration.parset
  parset-additions.d/default/StationPositions.parset
  parset-additions.d/default/StationStreams.parset
  DESTINATION etc/parset-additions.d/default)

INSTALL(CODE "FILE(MAKE_DIRECTORY \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/etc/parset-additions.d/override\")")
