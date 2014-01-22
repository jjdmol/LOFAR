# $Id$

lofar_add_bin_program(MACScheduler
  MACSchedulerMain.cc
  MACScheduler.cc
  ObsClaimer.cc)
                                                  
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/MACScheduler.conf.in
  ${CMAKE_CURRENT_BINARY_DIR}/MACScheduler.conf)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/MACScheduler.conf
  DESTINATION etc)
