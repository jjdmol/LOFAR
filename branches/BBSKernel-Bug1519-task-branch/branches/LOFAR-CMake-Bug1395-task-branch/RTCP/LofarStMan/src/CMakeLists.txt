# $Id: CMakeLists.txt 14290 2009-10-19 10:07:20Z mol $

include(LofarPackageVersion)

lofar_add_library(lofarstman 
  Package__Version.cc
  LofarStMan.cc
  LofarColumn.cc
  Register.cc
  )

lofar_add_executable(versionlofarstman versionlofarstman.cc)
