# $Id$

add_subdirectory(libs)

install(FILES
  claim.ctl
  monitorAlarms.ctl
  monitorStateChanges.ctl
  monitorStateReset.ctl
  monitorStationAlarms.ctl
  readStationConfigs.ctl
  readStationBGPconnections.ctl
  DESTINATION pvss/scripts)

