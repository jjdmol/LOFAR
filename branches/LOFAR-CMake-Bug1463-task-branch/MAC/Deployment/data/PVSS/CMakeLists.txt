# $Id$

install(PROGRAMS 
  create_db_files
  DESTINATION sbin)

file(GLOB pvss_dp_data
  PVSSDataPoints.base
  *.dpdef
  *.list)
install(FILES
  ${pvss_dp_data}
  DESTINATION pvss/dpdef)
