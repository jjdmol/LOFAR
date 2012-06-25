# $Id$

lofar_package(PVSS_Datapoints 1.0)

install(PROGRAMS 
  create_db_files
  DESTINATION sbin)

file(GLOB pvss_dp_data
  PVSSDataPoints.base
  CtrlDebug.dpl
  *.dpdef
  *.list)
install(FILES
  ${pvss_dp_data}
  DESTINATION pvss/dpdef)
