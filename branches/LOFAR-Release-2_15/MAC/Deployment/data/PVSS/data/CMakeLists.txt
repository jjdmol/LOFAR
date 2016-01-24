# $Id$

file(GLOB pvss_dp_data
  PVSSDataPoints.base
  CtrlDebug.dpl
  *.dpdef
  *.list)
install(FILES
  ${pvss_dp_data}
  DESTINATION pvss/dpdef)
