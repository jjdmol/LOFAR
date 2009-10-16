# $Id$

lofar_add_bin_program(CEPlogProcessor 
  CEPlogProcessorMain.cc
  CEPlogProcessor.cc)

install(FILES 
  CEPlogProcessor.conf 
  DESTINATION etc)
