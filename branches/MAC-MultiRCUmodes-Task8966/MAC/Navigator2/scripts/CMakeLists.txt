# $Id$

add_subdirectory(libs)

install(FILES
  claim.ctl
  gcf_cwd.ctl
  monitorAlarms.ctl
  monitorStationAlarms.ctl
  monitorStateChanges.ctl
  monitorStateReset.ctl
  readStationConfigs.ctl
  readStationConnections.ctl
  transferMPs.ctl
  DESTINATION pvss/scripts)
