# $Id$

lofar_package(StationTest 1.0)

# Install files matching regex pattern in current directory and below
install(DIRECTORY . 
  DESTINATION stationtest
  USE_SOURCE_PERMISSIONS
  FILES_MATCHING REGEX "(README\\.txt|\\.(gold|m|py|sh))$"
  PATTERN ".svn" EXCLUDE)
