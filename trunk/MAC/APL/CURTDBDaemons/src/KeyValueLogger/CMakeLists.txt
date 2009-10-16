# $Id$

lofar_add_bin_program(KeyValueLogger KeyValueLoggerMain.cc KeyValueLogger.cc)

install(FILES KeyValueLogger.conf DESTINATION etc)
