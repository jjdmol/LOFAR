# $Id$

lofar_add_bin_program(CTStartDaemon CTStartDaemonMain.cc CTStartDaemon.cc)
install(PROGRAMS startController.sh DESTINATION bin)
install(FILES CTStartDaemon.conf DESTINATION etc)
