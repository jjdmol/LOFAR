# $Id$

# Install in the test installation dir (allows usage in the ctest framework)
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/createbus.sh
  ${CMAKE_BINARY_DIR}/bin/createbus.sh @ONLY)


install(PROGRAMS
  createbus.sh
  createbusfromrange.sh
  DESTINATION bin)


