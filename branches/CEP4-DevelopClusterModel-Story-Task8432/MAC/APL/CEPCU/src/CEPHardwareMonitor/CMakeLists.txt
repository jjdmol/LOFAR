# $Id: CMakeLists.txt 18492 2011-07-14 21:07:51Z loose $

lofar_add_bin_program(CEPHardwareMonitor 
  CEPHardwareMonitorMain.cc
  ClusterMonitor.cc)

install(FILES
  CEPHardwareMonitor.conf
  DESTINATION etc)

lofar_add_sbin_scripts(zabbix_get)
