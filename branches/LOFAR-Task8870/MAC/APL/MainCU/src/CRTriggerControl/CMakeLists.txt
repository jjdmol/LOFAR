# $Id: CMakeLists.txt 14273 2009-10-16 10:08:29Z loose $

#lofar_package(CRTControl 1.0 DEPENDS MainCU CR_Protocol)

lofar_add_bin_program(CRTriggerControl
  TriggerControlMain.cc
  TriggerControl.cc)

lofar_add_bin_program(crctl
  crctl.cc)
  