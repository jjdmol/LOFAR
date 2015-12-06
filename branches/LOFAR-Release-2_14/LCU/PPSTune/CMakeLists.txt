# $Id: CMakeLists.txt 26657 2013-09-24 11:35:59Z schoenmakers $

lofar_package(PPSTune 1.0)

# Install files matching regex pattern in current directory and below
install(DIRECTORY . 
  DESTINATION sbin
  USE_SOURCE_PERMISSIONS
  FILES_MATCHING REGEX "\\.(py|sh)$"
  PATTERN ".svn" EXCLUDE)

