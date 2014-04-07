# $Id$

lofar_package(StationData 1.0)

# These files end up in ${prefix}/etc/StaticMetaData
file(GLOB staticmeta_data
  StationTables/*/*.conf)
install(FILES
  ${staticmeta_data}
  DESTINATION etc/StaticMetaData)

