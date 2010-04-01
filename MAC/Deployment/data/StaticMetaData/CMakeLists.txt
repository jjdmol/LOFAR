# $Id$

install(PROGRAMS 
  createFiles 
  DESTINATION sbin)

# These files end up in ${prefix}/etc
file(GLOB sysconf_data *.conf)
install(FILES
  ${sysconf_data}
  DESTINATION etc)

# These files end up in ${prefix}/etc/StaticMetaData
file(GLOB staticmeta_data 
  *.tmpl 
  *.test 
  *.dat 
  AntennaArrays/*.conf
  CableDelays/*.conf
  HBADeltas/*.conf)
install(FILES
  ${staticmeta_data}
  DESTINATION etc/StaticMetaData)

