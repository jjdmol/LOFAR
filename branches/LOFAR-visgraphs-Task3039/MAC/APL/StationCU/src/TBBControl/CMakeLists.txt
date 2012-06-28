# $Id$

lofar_add_bin_program(TBBControl 
  TBBControlMain.cc
  TBBControl.cc
  TBBObservation.cc)

lofar_add_bin_program(trigctl 
  trigctl.cc)

install(FILES
  TBBControl.conf
  DESTINATION etc)

