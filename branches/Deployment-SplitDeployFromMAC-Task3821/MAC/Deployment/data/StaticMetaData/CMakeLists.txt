# $Id$

lofar_package(StaticMetaData 1.0)

install(PROGRAMS 
  createFiles 
  DESTINATION sbin)

# These files end up in ${prefix}/etc
file(GLOB sysconf_data *.conf)
install(FILES
  ${sysconf_data}
  StationInfo.dat
  ControlInfo.dat
  DESTINATION etc)

# These files end up in ${prefix}/etc/StaticMetaData
file(GLOB staticmeta_data 
  *.tmpl 
  *.test 
  *.dat 
  TBBConnections.dat
  RSPConnections_CCU.dat)
install(FILES
  ${staticmeta_data}
  DESTINATION etc/StaticMetaData)
