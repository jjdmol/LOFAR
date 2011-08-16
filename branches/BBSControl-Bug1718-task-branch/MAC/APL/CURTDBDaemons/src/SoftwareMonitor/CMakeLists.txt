# $Id$

lofar_add_bin_program(SoftwareMonitor SoftwareMonitorMain.cc SoftwareMonitor.cc)

install(FILES SoftwareMonitor.conf DESTINATION etc)
