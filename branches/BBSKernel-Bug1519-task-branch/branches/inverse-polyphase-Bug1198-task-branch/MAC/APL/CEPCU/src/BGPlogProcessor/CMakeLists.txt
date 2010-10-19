# $Id$

lofar_add_bin_program(BGPlogProcessor 
  BGPlogProcessorMain.cc
  BGPlogProcessor.cc)

install(FILES 
  BGPlogProcessor.conf 
  DESTINATION etc)
