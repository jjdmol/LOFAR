# $Id$

lofar_package(StationTest 1.0)
lofar_add_package(MACTools)

# Install files matching regex pattern in current directory and below
install(DIRECTORY . 
  DESTINATION stationtest
  USE_SOURCE_PERMISSIONS
  FILES_MATCHING REGEX "(README\\.txt|\\.(gold|m|py|sh))$"
  PATTERN ".svn" EXCLUDE)

install(DIRECTORY ../../MAC/Tools/Power/
  DESTINATION stationtest/ec
  USE_SOURCE_PERMISSIONS
  FILES_MATCHING REGEX "(README\\.txt|\\.(gold|m|py|sh))$"
  PATTERN ".svn" EXCLUDE)
