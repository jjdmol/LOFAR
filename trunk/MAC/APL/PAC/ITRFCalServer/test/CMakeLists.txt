# $Id$

include(LofarCTest)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

#lofar_add_bin_program(CalTest CalTest.cc)
lofar_add_executable(tSubArrayMgr tSubArrayMgr.cc ../src/SubArrayMgr.cc)
lofar_add_executable(tCalServer tCalServer.cc)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/CalTest.conf.in 
  ${CMAKE_CURRENT_BINARY_DIR}/CalTest.conf)
  
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/CalTest.conf
    DESTINATION etc)
