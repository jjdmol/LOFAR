# $Id$

lofar_add_bin_program(LogClient LogClientMain.cc LogClient.cc)
install(FILES LogClient.conf DESTINATION etc)
