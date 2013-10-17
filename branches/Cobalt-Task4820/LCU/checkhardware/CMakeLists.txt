# $Id$

lofar_package(checkhardware 1.0)

# Install files matching regex pattern in current directory and below
install(DIRECTORY . 
  DESTINATION stationtest
  USE_SOURCE_PERMISSIONS
  FILES_MATCHING REGEX "(README\\.txt|\\.(gold|m|py|sh|conf))$"
  PATTERN ".svn" EXCLUDE)
