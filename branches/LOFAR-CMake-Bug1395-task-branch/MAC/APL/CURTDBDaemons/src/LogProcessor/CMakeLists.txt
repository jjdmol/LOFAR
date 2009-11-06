# $Id$

lofar_add_bin_program(LogProcessor LogProcessorMain.cc LogProcessor.cc)

install(FILES 
  LogProcessor.conf
  LogProcessor.log_prop
  DESTINATION etc)
