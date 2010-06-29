# $Id: CMakeLists.txt 14273 2009-10-16 10:08:29Z loose $

include(LofarCTest)

install(PROGRAMS
  tAnaBeamMgr
  tConverter
  tJ2000Converter.cc
  DESTINATION bin)

