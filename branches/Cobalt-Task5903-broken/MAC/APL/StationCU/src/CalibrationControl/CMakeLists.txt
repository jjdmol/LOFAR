# $Id$

lofar_add_bin_program(CalibrationControl 
  CalibrationControlMain.cc 
  CalibrationControl.cc)
  
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/CalibrationControl.conf.in
  ${CMAKE_CURRENT_BINARY_DIR}/CalibrationControl.conf)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/CalibrationControl.conf
  DESTINATION etc)
