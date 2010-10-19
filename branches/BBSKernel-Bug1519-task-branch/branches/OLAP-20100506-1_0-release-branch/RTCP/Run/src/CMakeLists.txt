# $Id: CMakeLists.txt 14273 2009-10-16 10:08:29Z loose $

lofar_add_bin_program(packetanalysis packetanalysis.c)

install(PROGRAMS 
  killOLAP
  swlevel
  BlueGeneControl.sh
  startOLAP.py
  commandOLAP.py
  runParset.py
  DESTINATION bin)

install(FILES
  OLAP.parset
  DESTINATION bin)

add_subdirectory(LOFAR)
add_subdirectory(util)
