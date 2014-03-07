# $Id: CMakeLists.txt 18492 2011-07-14 21:07:51Z loose $

lofar_add_bin_program(CEPHardwareMonitor 
  CEPHardwareMonitorMain.cc
  BlueGeneMonitor.cc
  ClusterMonitor.cc)

install(FILES
  CEPHardwareMonitor.conf
  DESTINATION etc)

install(PROGRAMS
  zabbix_get
  DESTINATION sbin)
