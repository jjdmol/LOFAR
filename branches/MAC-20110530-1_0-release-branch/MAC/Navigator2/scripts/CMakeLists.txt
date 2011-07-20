# $Id$

add_subdirectory(libs)

install(FILES
  claim.ctl
  monitorAlarms.ctl
  monitorStateChanges.ctl
  monitorStateReset.ctl
  readStationConfigs.ctl
  DESTINATION pvss/scripts)
