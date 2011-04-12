# $Id: CMakeLists.txt 14273 2009-10-16 10:08:29Z loose $

lofar_add_bin_program(packetanalysis packetanalysis.c)

install(PROGRAMS 
  killOLAP
  BlueGeneControl.sh
  watchlogs.sh
  startOLAP.py
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
