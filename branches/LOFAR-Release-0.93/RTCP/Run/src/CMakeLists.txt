# $Id: CMakeLists.txt 14273 2009-10-16 10:08:29Z loose $

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/locations.sh.in
  ${CMAKE_CURRENT_BINARY_DIR}/locations.sh
  @ONLY)

install(PROGRAMS 
  BGPPartition.sh
  CNProcessing.sh
  IONProcessing.sh
  controller.sh
  gracefullyStopBGProcessing.sh
  ${CMAKE_CURRENT_BINARY_DIR}/locations.sh
  watchlogs.sh
  commandOLAP.py
  runParset.py
  DESTINATION bin)

install(FILES
  BlueGeneControl.conf
  multitail-olap.conf
  OLAP.parset
  MAC+IP.dat
  RSPConnections.dat
  DESTINATION etc)

install(FILES
  multitail-olap.conf
  DESTINATION etc)

add_subdirectory(LOFAR)
add_subdirectory(util)
add_subdirectory(tools)
