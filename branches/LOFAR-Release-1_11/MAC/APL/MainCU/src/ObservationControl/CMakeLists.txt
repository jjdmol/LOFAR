# $Id$

lofar_add_bin_program(ObservationControl
  ObservationControlMain.cc
  ObservationControl.cc)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/ObservationControl.conf
  DESTINATION etc)
