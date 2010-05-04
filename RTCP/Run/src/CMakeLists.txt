# $Id: CMakeLists.txt 14273 2009-10-16 10:08:29Z loose $

add_executable(packetanalysis packetanalysis.c)

install(PROGRAMS 
  BlueGeneControl.sh
  packetanalysis
  startOLAP.py
  commandOLAP.py
  runParset.py
  DESTINATION bin)

add_subdirectory(LOFAR)
add_subdirectory(util)
