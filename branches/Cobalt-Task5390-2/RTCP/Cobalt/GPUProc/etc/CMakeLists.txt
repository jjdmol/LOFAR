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

# List of default/ config parsets:
# CMake cannot detect changes in this list, but for these files it doesn't need to.
file(GLOB _config_parsets
  "parset-additions.d/default/*.parset"
  "${CMAKE_CURRENT_BINARY_DIR}/parset-additions.d/default/*.parset")   # configured .parset.in
list(APPEND _config_parsets "parset-additions.d/default/HardwareUsed.parset.testsys")
install(FILES ${_config_parsets} DESTINATION etc/parset-additions.d/default)

# List of override/ config parsets:
# We have none. Create directory to allow post-install override/ parsets.
INSTALL(CODE "FILE(MAKE_DIRECTORY \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/etc/parset-additions.d/override\")")


# Install script to generate StationStreams.parset for the live test system.
install(PROGRAMS parset-additions.d/default/generateStationStreams.sh DESTINATION sbin)
