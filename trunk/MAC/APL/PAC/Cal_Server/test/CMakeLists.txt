# $Id$

include(LofarCTest)

lofar_add_bin_program(CalTest CalTest.cc)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/CalTest.conf.in 
  ${CMAKE_CURRENT_BINARY_DIR}/CalTest.conf)
  
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/CalTest.conf
    DESTINATION etc)
