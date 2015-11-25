# $Id$

lofar_add_bin_program(TBBControl 
  TBBControlMain.cc
  TBBControl.cc
  TBBObservation.cc)

lofar_add_bin_program(trigctl 
  trigctl.cc)

lofar_add_sbin_scripts(
  startTBB.sh
  stopTBB.sh)

install(FILES
  TBBControl.conf
  DESTINATION etc)

