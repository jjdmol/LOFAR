# $Id$

lofar_add_bin_program(TBBControl 
  TBBControlMain.cc
  TBBControl.cc
  TBBObservation.cc)

install(FILES
  TBBControl.conf
  DESTINATION etc)

