# $Id$

lofar_package(StaticMetaData 1.0)

lofar_add_sbin_scripts(createFiles)

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
  AntennaArrays/*.conf
  AntennaPos/*.conf
  CableDelays/*.conf
  iHBADeltas/*.conf
  AntennaFields/*.conf
  Attenuation/*.conf)
install(FILES
  ${staticmeta_data}
  DESTINATION etc/StaticMetaData)
