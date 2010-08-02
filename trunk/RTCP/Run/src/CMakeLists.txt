# $Id: CMakeLists.txt 14273 2009-10-16 10:08:29Z loose $

lofar_add_bin_program(packetanalysis packetanalysis.c)

install(PROGRAMS 
  killOLAP
  BlueGeneControl.sh
  watchlogs.sh
  startOLAP.py
  commandOLAP.py
  runParset.py
  testParset.py
  DESTINATION bin)

install(FILES
  multitail-olap.conf
  OLAP.parset
  DESTINATION etc)

add_subdirectory(LOFAR)
add_subdirectory(util)
